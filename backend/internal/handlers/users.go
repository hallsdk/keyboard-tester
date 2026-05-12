package handlers

import (
	"database/sql"
	"encoding/json"
	"errors"
	"net/http"
	"strconv"
	"strings"

	"github.com/go-chi/chi/v5"
	"golang.org/x/crypto/bcrypt"

	"github.com/hallsdk/ktester-backend/internal/auth"
	"github.com/hallsdk/ktester-backend/internal/models"
)

type UserHandler struct {
	db *sql.DB
}

func NewUserHandler(db *sql.DB) *UserHandler {
	return &UserHandler{db: db}
}

type userOut struct {
	ID               int64   `json:"id"`
	Username         string  `json:"username"`
	Role             string  `json:"role"`
	Status           string  `json:"status"`
	PendingFactoryID *int64  `json:"pending_factory_id,omitempty"`
	CreatedAt        string  `json:"created_at"`
	LastLogin        *string `json:"last_login,omitempty"`
	FactoryIDs       []int64 `json:"factory_ids"`
}

// List:
//   - super_admin sees ALL users
//   - admin sees users sharing at least one factory with them (super_admins hidden)
func (h *UserHandler) List(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var (
		rows *sql.Rows
		err  error
	)
	if c.Role == models.RoleSuperAdmin {
		rows, err = h.db.Query(`SELECT id, username, role, status, pending_factory_id, created_at, last_login FROM users WHERE status='active' ORDER BY id`)
	} else {
		rows, err = h.db.Query(`
			SELECT DISTINCT u.id, u.username, u.role, u.status, u.pending_factory_id, u.created_at, u.last_login
			FROM users u
			JOIN user_factories uf ON uf.user_id = u.id
			WHERE u.role != 'super_admin' AND u.status = 'active'
			  AND uf.factory_id IN (SELECT factory_id FROM user_factories WHERE user_id = ?)
			ORDER BY u.id`, c.UserID)
	}
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer rows.Close()
	out := []userOut{}
	ids := []int64{}
	for rows.Next() {
		var u userOut
		var last sql.NullString
		var pfid sql.NullInt64
		if err := rows.Scan(&u.ID, &u.Username, &u.Role, &u.Status, &pfid, &u.CreatedAt, &last); err != nil {
			writeErr(w, 500, "scan error")
			return
		}
		if last.Valid {
			s := last.String
			u.LastLogin = &s
		}
		if pfid.Valid {
			v := pfid.Int64
			u.PendingFactoryID = &v
		}
		u.FactoryIDs = []int64{}
		out = append(out, u)
		ids = append(ids, u.ID)
	}
	// Attach factory_ids in one extra query.
	if len(ids) > 0 {
		idx := make(map[int64]*userOut, len(out))
		for i := range out {
			idx[out[i].ID] = &out[i]
		}
		// Build IN clause.
		placeholders := strings.Repeat("?,", len(ids))
		placeholders = placeholders[:len(placeholders)-1]
		args := make([]any, len(ids))
		for i, id := range ids {
			args[i] = id
		}
		q := `SELECT user_id, factory_id FROM user_factories WHERE user_id IN (` + placeholders + `)`
		r2, err := h.db.Query(q, args...)
		if err == nil {
			defer r2.Close()
			for r2.Next() {
				var uid, fid int64
				if err := r2.Scan(&uid, &fid); err == nil {
					if u := idx[uid]; u != nil {
						u.FactoryIDs = append(u.FactoryIDs, fid)
					}
				}
			}
		}
	}
	writeJSON(w, 200, out)
}

type createUserInput struct {
	Username   string  `json:"username"`
	Password   string  `json:"password"`
	Role       string  `json:"role"`
	FactoryIDs []int64 `json:"factory_ids"`
}

// Create: super_admin can create any role; admin can create 'user' inside their own factory only.
func (h *UserHandler) Create(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var in createUserInput
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	if !validRole(in.Role) {
		writeErr(w, 400, "invalid role")
		return
	}
	in.Username = strings.TrimSpace(in.Username)
	if len(in.Username) < 3 || len(in.Password) < 6 {
		writeErr(w, 400, "username>=3 password>=6")
		return
	}
	// admin can only create 'user' role in their own factory.
	if c.Role == models.RoleAdmin {
		if in.Role != models.RoleUser {
			writeErr(w, 403, "admin can only create regular users")
			return
		}
		if len(in.FactoryIDs) != 1 {
			writeErr(w, 400, "must specify exactly one factory_id")
			return
		}
		// Must be admin's own factory.
		var n int
		if err := h.db.QueryRow(`SELECT COUNT(*) FROM user_factories WHERE user_id=? AND factory_id=?`,
			c.UserID, in.FactoryIDs[0]).Scan(&n); err != nil || n == 0 {
			writeErr(w, 403, "not your factory")
			return
		}
	}
	if in.Role == models.RoleUser && len(in.FactoryIDs) != 1 {
		writeErr(w, 400, "user role requires exactly one factory_id")
		return
	}
	if in.Role == models.RoleAdmin && len(in.FactoryIDs) == 0 {
		writeErr(w, 400, "admin role requires at least one factory_id")
		return
	}
	hash, err := bcrypt.GenerateFromPassword([]byte(in.Password), bcrypt.DefaultCost)
	if err != nil {
		writeErr(w, 500, "hash error")
		return
	}
	tx, err := h.db.Begin()
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer tx.Rollback()
	res, err := tx.Exec(`INSERT INTO users (username, password_hash, role, status) VALUES (?,?,?,'active')`,
		in.Username, string(hash), in.Role)
	if err != nil {
		if strings.Contains(err.Error(), "UNIQUE") {
			writeErr(w, 409, "username exists")
			return
		}
		writeErr(w, 500, "db error")
		return
	}
	id, _ := res.LastInsertId()
	for _, fid := range in.FactoryIDs {
		if _, err := tx.Exec(`INSERT INTO user_factories (user_id, factory_id) VALUES (?,?)`, id, fid); err != nil {
			writeErr(w, 400, "invalid factory_id: "+strconv.FormatInt(fid, 10))
			return
		}
	}
	if err := tx.Commit(); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "user.create", strconv.FormatInt(id, 10),
		in.Username+" as "+in.Role)
	writeJSON(w, 201, map[string]any{"id": id})
}

// Update: super_admin only. Change role / reset password.
func (h *UserHandler) Update(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	var in struct {
		Role     *string `json:"role"`
		Password *string `json:"password"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	sets := []string{}
	args := []any{}
	if in.Role != nil {
		if !validRole(*in.Role) {
			writeErr(w, 400, "invalid role")
			return
		}
		sets = append(sets, "role = ?")
		args = append(args, *in.Role)
	}
	if in.Password != nil {
		if len(*in.Password) < 6 {
			writeErr(w, 400, "password too short")
			return
		}
		hash, _ := bcrypt.GenerateFromPassword([]byte(*in.Password), bcrypt.DefaultCost)
		sets = append(sets, "password_hash = ?")
		args = append(args, string(hash))
	}
	if len(sets) == 0 {
		writeErr(w, 400, "nothing to update")
		return
	}
	args = append(args, id)
	q := "UPDATE users SET " + strings.Join(sets, ", ") + " WHERE id = ?"
	if _, err := h.db.Exec(q, args...); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "user.update", strconv.FormatInt(id, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// Delete: super_admin only; cannot delete yourself.
func (h *UserHandler) Delete(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	if id == c.UserID {
		writeErr(w, 400, "cannot delete yourself")
		return
	}
	// Safety: cannot leave zero super_admins.
	var role string
	if err := h.db.QueryRow(`SELECT role FROM users WHERE id=?`, id).Scan(&role); err != nil {
		if errors.Is(err, sql.ErrNoRows) {
			writeErr(w, 404, "not found")
			return
		}
		writeErr(w, 500, "db error")
		return
	}
	if role == models.RoleSuperAdmin {
		var n int
		_ = h.db.QueryRow(`SELECT COUNT(*) FROM users WHERE role='super_admin'`).Scan(&n)
		if n <= 1 {
			writeErr(w, 400, "cannot delete last super_admin")
			return
		}
	}
	if _, err := h.db.Exec(`DELETE FROM users WHERE id=?`, id); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "user.delete", strconv.FormatInt(id, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// ListPending returns pending-registration users whose pending_factory_id belongs to the caller's factories.
// super_admin sees all pending.
func (h *UserHandler) ListPending(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var (
		rows *sql.Rows
		err  error
	)
	if c.Role == models.RoleSuperAdmin {
		rows, err = h.db.Query(`SELECT id, username, role, status, pending_factory_id, created_at, last_login FROM users WHERE status='pending' ORDER BY id`)
	} else {
		rows, err = h.db.Query(`
			SELECT id, username, role, status, pending_factory_id, created_at, last_login
			FROM users
			WHERE status='pending'
			  AND pending_factory_id IN (SELECT factory_id FROM user_factories WHERE user_id=?)
			ORDER BY id`, c.UserID)
	}
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer rows.Close()
	out := []userOut{}
	for rows.Next() {
		var u userOut
		var last sql.NullString
		var pfid sql.NullInt64
		if err := rows.Scan(&u.ID, &u.Username, &u.Role, &u.Status, &pfid, &u.CreatedAt, &last); err != nil {
			writeErr(w, 500, "scan error")
			return
		}
		if last.Valid {
			s := last.String
			u.LastLogin = &s
		}
		if pfid.Valid {
			v := pfid.Int64
			u.PendingFactoryID = &v
		}
		u.FactoryIDs = []int64{}
		out = append(out, u)
	}
	writeJSON(w, 200, out)
}

// Approve activates a pending user and assigns them to the factory they applied to.
// Admin can only approve users whose pending_factory_id is in their own factories.
func (h *UserHandler) Approve(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	var pfid sql.NullInt64
	var status string
	if err := h.db.QueryRow(`SELECT status, pending_factory_id FROM users WHERE id=?`, id).Scan(&status, &pfid); errors.Is(err, sql.ErrNoRows) {
		writeErr(w, 404, "user not found")
		return
	} else if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	if status != "pending" {
		writeErr(w, 400, "user is not pending")
		return
	}
	if !pfid.Valid {
		writeErr(w, 400, "user has no pending factory")
		return
	}
	factoryID := pfid.Int64
	// Non-super_admin must own the factory.
	if c.Role != models.RoleSuperAdmin {
		var n int
		if err := h.db.QueryRow(`SELECT COUNT(*) FROM user_factories WHERE user_id=? AND factory_id=?`,
			c.UserID, factoryID).Scan(&n); err != nil || n == 0 {
			writeErr(w, 403, "not your factory")
			return
		}
	}
	tx, err := h.db.Begin()
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer tx.Rollback()
	if _, err := tx.Exec(`UPDATE users SET status='active', pending_factory_id=NULL WHERE id=?`, id); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	if _, err := tx.Exec(`INSERT OR IGNORE INTO user_factories (user_id, factory_id) VALUES (?,?)`, id, factoryID); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	if err := tx.Commit(); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "user.approve", strconv.FormatInt(id, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// Reject deletes a pending user. Admin can only reject users in their own factories.
func (h *UserHandler) Reject(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	var pfid sql.NullInt64
	var status string
	if err := h.db.QueryRow(`SELECT status, pending_factory_id FROM users WHERE id=?`, id).Scan(&status, &pfid); errors.Is(err, sql.ErrNoRows) {
		writeErr(w, 404, "user not found")
		return
	} else if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	if status != "pending" {
		writeErr(w, 400, "user is not pending")
		return
	}
	if c.Role != models.RoleSuperAdmin {
		if !pfid.Valid {
			writeErr(w, 403, "cannot reject")
			return
		}
		var n int
		if err := h.db.QueryRow(`SELECT COUNT(*) FROM user_factories WHERE user_id=? AND factory_id=?`,
			c.UserID, pfid.Int64).Scan(&n); err != nil || n == 0 {
			writeErr(w, 403, "not your factory")
			return
		}
	}
	if _, err := h.db.Exec(`DELETE FROM users WHERE id=? AND status='pending'`, id); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "user.reject", strconv.FormatInt(id, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

func validRole(r string) bool {
	switch r {
	case models.RoleUser, models.RoleAdmin, models.RoleSuperAdmin:
		return true
	}
	return false
}

func audit(db *sql.DB, uid int64, action, target, detail string) {
	_, _ = db.Exec(`INSERT INTO audit_log (user_id, action, target, detail) VALUES (?,?,?,?)`,
		uid, action, target, detail)
}
