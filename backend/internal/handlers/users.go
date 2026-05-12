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
	ID        int64   `json:"id"`
	Username  string  `json:"username"`
	Role      string  `json:"role"`
	CreatedAt string  `json:"created_at"`
	LastLogin *string `json:"last_login,omitempty"`
}

// List:
//   - super_admin sees ALL users
//   - admin sees user + admin (super_admins are HIDDEN by design)
func (h *UserHandler) List(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	q := `SELECT id, username, role, created_at, last_login FROM users`
	args := []any{}
	if c.Role != models.RoleSuperAdmin {
		q += ` WHERE role != 'super_admin'`
	}
	q += ` ORDER BY id`
	rows, err := h.db.Query(q, args...)
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer rows.Close()
	out := []userOut{}
	for rows.Next() {
		var u userOut
		var last sql.NullString
		if err := rows.Scan(&u.ID, &u.Username, &u.Role, &u.CreatedAt, &last); err != nil {
			writeErr(w, 500, "scan error")
			return
		}
		if last.Valid {
			s := last.String
			u.LastLogin = &s
		}
		out = append(out, u)
	}
	writeJSON(w, 200, out)
}

type createUserInput struct {
	Username string `json:"username"`
	Password string `json:"password"`
	Role     string `json:"role"`
}

// Create: super_admin only. Can pick any role including super_admin.
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
	hash, err := bcrypt.GenerateFromPassword([]byte(in.Password), bcrypt.DefaultCost)
	if err != nil {
		writeErr(w, 500, "hash error")
		return
	}
	res, err := h.db.Exec(`INSERT INTO users (username, password_hash, role) VALUES (?,?,?)`,
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
