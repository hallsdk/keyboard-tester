package handlers

import (
	"database/sql"
	"encoding/json"
	"errors"
	"net/http"
	"strconv"
	"strings"

	"github.com/go-chi/chi/v5"

	"github.com/hallsdk/ktester-backend/internal/auth"
	"github.com/hallsdk/ktester-backend/internal/models"
)

type FactoryHandler struct {
	db *sql.DB
}

func NewFactoryHandler(db *sql.DB) *FactoryHandler {
	return &FactoryHandler{db: db}
}

// List: super_admin sees all, admin sees their own factories, user sees their factory.
func (h *FactoryHandler) List(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var (
		rows *sql.Rows
		err  error
	)
	if c.Role == models.RoleSuperAdmin {
		rows, err = h.db.Query(`SELECT id, name, created_at FROM factories ORDER BY id`)
	} else {
		rows, err = h.db.Query(`
			SELECT f.id, f.name, f.created_at
			FROM factories f
			JOIN user_factories uf ON uf.factory_id = f.id
			WHERE uf.user_id = ?
			ORDER BY f.id`, c.UserID)
	}
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer rows.Close()
	out := []models.Factory{}
	for rows.Next() {
		var f models.Factory
		if err := rows.Scan(&f.ID, &f.Name, &f.CreatedAt); err != nil {
			writeErr(w, 500, "scan error")
			return
		}
		out = append(out, f)
	}
	writeJSON(w, 200, out)
}

// Create: super_admin only.
func (h *FactoryHandler) Create(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var in struct {
		Name string `json:"name"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	in.Name = strings.TrimSpace(in.Name)
	if in.Name == "" {
		writeErr(w, 400, "name required")
		return
	}
	res, err := h.db.Exec(`INSERT INTO factories (name) VALUES (?)`, in.Name)
	if err != nil {
		if strings.Contains(err.Error(), "UNIQUE") {
			writeErr(w, 409, "factory name exists")
			return
		}
		writeErr(w, 500, "db error")
		return
	}
	id, _ := res.LastInsertId()
	audit(h.db, c.UserID, "factory.create", strconv.FormatInt(id, 10), in.Name)
	writeJSON(w, 201, map[string]any{"id": id})
}

// Update: super_admin only — rename.
func (h *FactoryHandler) Update(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	var in struct {
		Name string `json:"name"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	in.Name = strings.TrimSpace(in.Name)
	if in.Name == "" {
		writeErr(w, 400, "name required")
		return
	}
	if _, err := h.db.Exec(`UPDATE factories SET name=? WHERE id=?`, in.Name, id); err != nil {
		if strings.Contains(err.Error(), "UNIQUE") {
			writeErr(w, 409, "factory name exists")
			return
		}
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "factory.update", strconv.FormatInt(id, 10), in.Name)
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// Delete: super_admin only. Devices keep existing (factory_id becomes NULL).
func (h *FactoryHandler) Delete(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	if _, err := h.db.Exec(`DELETE FROM factories WHERE id=?`, id); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "factory.delete", strconv.FormatInt(id, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// SetUserFactories: super_admin assigns a user to one or more factories.
//   PUT /api/users/{id}/factories  body: {"factory_ids": [1,2]}
// For role=user, must be exactly 1. For role=admin, >=1.
func (h *FactoryHandler) SetUserFactories(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	uid, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	var in struct {
		FactoryIDs []int64 `json:"factory_ids"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	var role string
	if err := h.db.QueryRow(`SELECT role FROM users WHERE id=?`, uid).Scan(&role); err != nil {
		if errors.Is(err, sql.ErrNoRows) {
			writeErr(w, 404, "user not found")
			return
		}
		writeErr(w, 500, "db error")
		return
	}
	if role == models.RoleSuperAdmin {
		writeErr(w, 400, "super_admin has access to all factories")
		return
	}
	if role == models.RoleUser && len(in.FactoryIDs) != 1 {
		writeErr(w, 400, "regular user must belong to exactly one factory")
		return
	}
	if len(in.FactoryIDs) == 0 {
		writeErr(w, 400, "at least one factory required")
		return
	}
	tx, err := h.db.Begin()
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer tx.Rollback()
	if _, err := tx.Exec(`DELETE FROM user_factories WHERE user_id=?`, uid); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	for _, fid := range in.FactoryIDs {
		if _, err := tx.Exec(`INSERT INTO user_factories (user_id, factory_id) VALUES (?,?)`, uid, fid); err != nil {
			writeErr(w, 400, "invalid factory_id: "+strconv.FormatInt(fid, 10))
			return
		}
	}
	if err := tx.Commit(); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "user.set_factories", strconv.FormatInt(uid, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// GetUserFactories: returns the list of factory IDs the user belongs to.
//   GET /api/users/{id}/factories
func (h *FactoryHandler) GetUserFactories(w http.ResponseWriter, r *http.Request) {
	uid, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	rows, err := h.db.Query(`SELECT factory_id FROM user_factories WHERE user_id=?`, uid)
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer rows.Close()
	out := []int64{}
	for rows.Next() {
		var id int64
		if err := rows.Scan(&id); err != nil {
			writeErr(w, 500, "scan error")
			return
		}
		out = append(out, id)
	}
	writeJSON(w, 200, map[string]any{"factory_ids": out})
}

// SetUserDevices: admin sets which devices a regular user can see (whitelist).
//   PUT /api/users/{id}/devices  body: {"device_ids": [1,2,3]}
// An empty array means "all devices of the user's factory" (no per-user restriction).
// Admin can only operate on users in factories they also belong to.
func (h *FactoryHandler) SetUserDevices(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	uid, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	var in struct {
		DeviceIDs []int64 `json:"device_ids"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	// Admins: must share at least one factory with the target user.
	if c.Role == models.RoleAdmin {
		var n int
		if err := h.db.QueryRow(`
			SELECT COUNT(*) FROM user_factories a
			JOIN user_factories b ON a.factory_id = b.factory_id
			WHERE a.user_id = ? AND b.user_id = ?`, c.UserID, uid).Scan(&n); err != nil {
			writeErr(w, 500, "db error")
			return
		}
		if n == 0 {
			writeErr(w, 403, "user is not in your factory")
			return
		}
	}
	tx, err := h.db.Begin()
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer tx.Rollback()
	if _, err := tx.Exec(`DELETE FROM user_devices WHERE user_id=?`, uid); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	for _, did := range in.DeviceIDs {
		if _, err := tx.Exec(`INSERT INTO user_devices (user_id, device_id) VALUES (?,?)`, uid, did); err != nil {
			writeErr(w, 400, "invalid device_id: "+strconv.FormatInt(did, 10))
			return
		}
	}
	if err := tx.Commit(); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	audit(h.db, c.UserID, "user.set_devices", strconv.FormatInt(uid, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// GetUserDevices: returns the device IDs explicitly whitelisted for a user.
//   GET /api/users/{id}/devices
func (h *FactoryHandler) GetUserDevices(w http.ResponseWriter, r *http.Request) {
	uid, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	rows, err := h.db.Query(`SELECT device_id FROM user_devices WHERE user_id=?`, uid)
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer rows.Close()
	out := []int64{}
	for rows.Next() {
		var id int64
		if err := rows.Scan(&id); err != nil {
			writeErr(w, 500, "scan error")
			return
		}
		out = append(out, id)
	}
	writeJSON(w, 200, map[string]any{"device_ids": out})
}
