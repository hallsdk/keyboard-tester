package handlers

import (
	"database/sql"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/go-chi/chi/v5"

	"github.com/hallsdk/ktester-backend/internal/auth"
	"github.com/hallsdk/ktester-backend/internal/models"
)

type DeviceHandler struct {
	db         *sql.DB
	layoutsDir string
}

func NewDeviceHandler(db *sql.DB, dir string) *DeviceHandler {
	return &DeviceHandler{db: db, layoutsDir: dir}
}

var vidpidRe = regexp.MustCompile(`^0x[0-9A-F]{4}$`)

func normHex(s string) string {
	s = strings.TrimSpace(s)
	lower := strings.ToLower(s)
	if !strings.HasPrefix(lower, "0x") {
		lower = "0x" + lower
	}
	digits := strings.TrimPrefix(lower, "0x")
	return "0x" + strings.ToUpper(digits)
}

func (h *DeviceHandler) List(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var (
		rows *sql.Rows
		err  error
	)
	switch c.Role {
	case models.RoleSuperAdmin:
		rows, err = h.db.Query(`
			SELECT id, factory_id, vid, pid, name, description, layout_file, created_by, created_at, updated_at
			FROM devices ORDER BY vid, pid`)
	case models.RoleAdmin:
		rows, err = h.db.Query(`
			SELECT d.id, d.factory_id, d.vid, d.pid, d.name, d.description, d.layout_file, d.created_by, d.created_at, d.updated_at
			FROM devices d
			WHERE d.factory_id IN (SELECT factory_id FROM user_factories WHERE user_id = ?)
			ORDER BY d.vid, d.pid`, c.UserID)
	default: // RoleUser
		// If a factory-level whitelist exists, use it; otherwise show all devices of the user's factory.
		rows, err = h.db.Query(`
			SELECT d.id, d.factory_id, d.vid, d.pid, d.name, d.description, d.layout_file, d.created_by, d.created_at, d.updated_at
			FROM devices d
			WHERE d.factory_id IN (SELECT factory_id FROM user_factories WHERE user_id = ?)
			  AND (
			        NOT EXISTS (SELECT 1 FROM factory_visible_devices WHERE factory_id = d.factory_id)
			        OR d.id IN (SELECT device_id FROM factory_visible_devices WHERE factory_id = d.factory_id)
			      )
			ORDER BY d.vid, d.pid`, c.UserID)
	}
	if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	defer rows.Close()
	out := []models.Device{}
	for rows.Next() {
		var d models.Device
		var fid, cb sql.NullInt64
		if err := rows.Scan(&d.ID, &fid, &d.VID, &d.PID, &d.Name, &d.Description,
			&d.LayoutFile, &cb, &d.CreatedAt, &d.UpdatedAt); err != nil {
			writeErr(w, 500, "scan error")
			return
		}
		if fid.Valid {
			d.FactoryID = &fid.Int64
		}
		if cb.Valid {
			d.CreatedBy = &cb.Int64
		}
		out = append(out, d)
	}
	writeJSON(w, 200, out)
}

func (h *DeviceHandler) Get(w http.ResponseWriter, r *http.Request) {
	vid := normHex(chi.URLParam(r, "vid"))
	pid := normHex(chi.URLParam(r, "pid"))
	if !vidpidRe.MatchString(vid) || !vidpidRe.MatchString(pid) {
		writeErr(w, 400, "invalid vid/pid")
		return
	}
	c := auth.ClaimsFrom(r)
	var d models.Device
	var fid, cb sql.NullInt64
	var err error
	switch c.Role {
	case models.RoleSuperAdmin:
		err = h.db.QueryRow(`
			SELECT id, factory_id, vid, pid, name, description, layout_file, created_by, created_at, updated_at
			FROM devices WHERE vid = ? AND pid = ? LIMIT 1`, vid, pid,
		).Scan(&d.ID, &fid, &d.VID, &d.PID, &d.Name, &d.Description, &d.LayoutFile, &cb, &d.CreatedAt, &d.UpdatedAt)
	default:
		err = h.db.QueryRow(`
			SELECT id, factory_id, vid, pid, name, description, layout_file, created_by, created_at, updated_at
			FROM devices
			WHERE vid = ? AND pid = ?
			  AND factory_id IN (SELECT factory_id FROM user_factories WHERE user_id = ?)
			LIMIT 1`, vid, pid, c.UserID,
		).Scan(&d.ID, &fid, &d.VID, &d.PID, &d.Name, &d.Description, &d.LayoutFile, &cb, &d.CreatedAt, &d.UpdatedAt)
	}
	if errors.Is(err, sql.ErrNoRows) {
		writeErr(w, 404, "not found")
		return
	} else if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	if fid.Valid {
		d.FactoryID = &fid.Int64
	}
	if cb.Valid {
		d.CreatedBy = &cb.Int64
	}
	writeJSON(w, 200, d)
}

func (h *DeviceHandler) DownloadLayout(w http.ResponseWriter, r *http.Request) {
	vid := normHex(chi.URLParam(r, "vid"))
	pid := normHex(chi.URLParam(r, "pid"))
	if !vidpidRe.MatchString(vid) || !vidpidRe.MatchString(pid) {
		writeErr(w, 400, "invalid vid/pid")
		return
	}
	cl := auth.ClaimsFrom(r)
	var (
		name string
		id   int64
		fid  sql.NullInt64
	)
	var err error
	if cl.Role == models.RoleSuperAdmin {
		err = h.db.QueryRow(
			`SELECT id, layout_file, factory_id FROM devices WHERE vid=? AND pid=? LIMIT 1`, vid, pid).
			Scan(&id, &name, &fid)
	} else {
		err = h.db.QueryRow(`
			SELECT id, layout_file, factory_id FROM devices
			WHERE vid=? AND pid=?
			  AND factory_id IN (SELECT factory_id FROM user_factories WHERE user_id=?)
			LIMIT 1`, vid, pid, cl.UserID).
			Scan(&id, &name, &fid)
	}
	if errors.Is(err, sql.ErrNoRows) {
		writeErr(w, 404, "not found")
		return
	} else if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	full := filepath.Join(h.layoutsDir, filepath.Clean("/"+name))
	if !strings.HasPrefix(full, filepath.Clean(h.layoutsDir)+string(os.PathSeparator)) &&
		full != filepath.Clean(h.layoutsDir) {
		writeErr(w, 400, "bad path")
		return
	}
	f, err := os.Open(full)
	if err != nil {
		writeErr(w, 404, "layout file missing")
		return
	}
	defer f.Close()
	w.Header().Set("Content-Type", "application/octet-stream")
	w.Header().Set("Content-Disposition",
		fmt.Sprintf(`attachment; filename="%s"`, filepath.Base(name)))
	_, _ = io.Copy(w, f)
}

type createDeviceInput struct {
	FactoryID   int64  `json:"factory_id"`
	VID         string `json:"vid"`
	PID         string `json:"pid"`
	Name        string `json:"name"`
	Description string `json:"description"`
	LayoutFile  string `json:"layout_file"`        // filename only
	LayoutBody  string `json:"layout_body"`        // optional inline file content
}

func (h *DeviceHandler) Create(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var in createDeviceInput
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	vid, pid := normHex(in.VID), normHex(in.PID)
	if !vidpidRe.MatchString(vid) || !vidpidRe.MatchString(pid) {
		writeErr(w, 400, "invalid vid/pid (need 0xHHHH)")
		return
	}
	if strings.TrimSpace(in.Name) == "" {
		writeErr(w, 400, "name required")
		return
	}
	if in.LayoutFile == "" {
		writeErr(w, 400, "layout_file required")
		return
	}
	if in.FactoryID == 0 {
		writeErr(w, 400, "factory_id required")
		return
	}
	// Admin can only create devices inside one of their own factories.
	if c.Role == models.RoleAdmin {
		var n int
		if err := h.db.QueryRow(`SELECT COUNT(*) FROM user_factories WHERE user_id=? AND factory_id=?`,
			c.UserID, in.FactoryID).Scan(&n); err != nil {
			writeErr(w, 500, "db error")
			return
		}
		if n == 0 {
			writeErr(w, 403, "not your factory")
			return
		}
	}
	if err := h.writeLayout(in.LayoutFile, in.LayoutBody); err != nil {
		writeErr(w, 400, "layout write: "+err.Error())
		return
	}
	res, err := h.db.Exec(`
		INSERT INTO devices (factory_id, vid, pid, name, description, layout_file, created_by, updated_at)
		VALUES (?,?,?,?,?,?,?,CURRENT_TIMESTAMP)`,
		in.FactoryID, vid, pid, strings.TrimSpace(in.Name), in.Description, in.LayoutFile, c.UserID)
	if err != nil {
		if strings.Contains(err.Error(), "UNIQUE") {
			writeErr(w, 409, "该工厂下此 VID/PID 已存在")
			return
		}
		writeErr(w, 500, "db error: "+err.Error())
		return
	}
	id, _ := res.LastInsertId()
	// If this factory already has a visible-device whitelist, auto-add the new device to it
	// so employees can see it immediately without manual reconfiguration.
	var wlCount int
	if err := h.db.QueryRow(`SELECT COUNT(*) FROM factory_visible_devices WHERE factory_id=?`, in.FactoryID).Scan(&wlCount); err == nil && wlCount > 0 {
		_, _ = h.db.Exec(`INSERT OR IGNORE INTO factory_visible_devices (factory_id, device_id) VALUES (?,?)`, in.FactoryID, id)
	}
	h.audit(c.UserID, "device.create", fmt.Sprintf("%s-%s", vid, pid), in.Name)
	writeJSON(w, 201, map[string]any{"id": id})
}

func (h *DeviceHandler) Update(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	if !h.canMutateDevice(c, id) {
		writeErr(w, 403, "forbidden")
		return
	}
	var in struct {
		FactoryID   *int64  `json:"factory_id"`
		Name        *string `json:"name"`
		Description *string `json:"description"`
		LayoutFile  *string `json:"layout_file"`
		LayoutBody  *string `json:"layout_body"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	sets := []string{}
	args := []any{}
	if in.FactoryID != nil {
		// Only super_admin may move a device across factories.
		if c.Role != models.RoleSuperAdmin {
			writeErr(w, 403, "only super_admin can change factory_id")
			return
		}
		sets = append(sets, "factory_id = ?")
		args = append(args, *in.FactoryID)
	}
	if in.Name != nil {
		sets = append(sets, "name = ?")
		args = append(args, *in.Name)
	}
	if in.Description != nil {
		sets = append(sets, "description = ?")
		args = append(args, *in.Description)
	}
	if in.LayoutFile != nil {
		body := ""
		if in.LayoutBody != nil {
			body = *in.LayoutBody
		}
		if err := h.writeLayout(*in.LayoutFile, body); err != nil {
			writeErr(w, 400, "layout write: "+err.Error())
			return
		}
		sets = append(sets, "layout_file = ?")
		args = append(args, *in.LayoutFile)
	}
	if len(sets) == 0 {
		writeErr(w, 400, "nothing to update")
		return
	}
	sets = append(sets, "updated_at = ?")
	args = append(args, time.Now())
	args = append(args, id)
	q := "UPDATE devices SET " + strings.Join(sets, ", ") + " WHERE id = ?"
	if _, err := h.db.Exec(q, args...); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	h.audit(c.UserID, "device.update", strconv.FormatInt(id, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// UploadLayout: PUT /api/devices/{id}/layout, raw body = file content,
// query param ?filename=xxx.json optional (defaults to current layout_file).
func (h *DeviceHandler) UploadLayout(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	if !h.canMutateDevice(c, id) {
		writeErr(w, 403, "forbidden")
		return
	}
	var current string
	if err := h.db.QueryRow(`SELECT layout_file FROM devices WHERE id=?`, id).Scan(&current); err != nil {
		writeErr(w, 404, "device not found")
		return
	}
	name := r.URL.Query().Get("filename")
	if name == "" {
		name = current
	}
	body, err := io.ReadAll(io.LimitReader(r.Body, 4*1024*1024))
	if err != nil {
		writeErr(w, 400, "read body")
		return
	}
	if err := h.writeLayoutBytes(name, body); err != nil {
		writeErr(w, 400, "write: "+err.Error())
		return
	}
	if _, err := h.db.Exec(
		`UPDATE devices SET layout_file=?, updated_at=CURRENT_TIMESTAMP WHERE id=?`,
		name, id); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	h.audit(c.UserID, "device.upload_layout", strconv.FormatInt(id, 10), name)
	writeJSON(w, 200, map[string]string{"status": "ok", "layout_file": name})
}

func (h *DeviceHandler) Delete(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	id, err := strconv.ParseInt(chi.URLParam(r, "id"), 10, 64)
	if err != nil {
		writeErr(w, 400, "bad id")
		return
	}
	if !h.canMutateDevice(c, id) {
		writeErr(w, 403, "forbidden")
		return
	}
	if _, err := h.db.Exec(`DELETE FROM devices WHERE id=?`, id); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	h.audit(c.UserID, "device.delete", strconv.FormatInt(id, 10), "")
	writeJSON(w, 200, map[string]string{"status": "ok"})
}

// ---- helpers ----

var safeName = regexp.MustCompile(`^[A-Za-z0-9._\-/]+$`)

func (h *DeviceHandler) writeLayout(name, body string) error {
	if body == "" {
		// Caller will upload via PUT /layout afterwards; just ensure the
		// directory exists. We do NOT create an empty file here, but we
		// require the file to exist before allowing fetch.
		return nil
	}
	return h.writeLayoutBytes(name, []byte(body))
}

func (h *DeviceHandler) writeLayoutBytes(name string, body []byte) error {
	if !safeName.MatchString(name) || strings.Contains(name, "..") {
		return fmt.Errorf("unsafe filename")
	}
	full := filepath.Join(h.layoutsDir, filepath.Clean("/"+name))
	if !strings.HasPrefix(full, filepath.Clean(h.layoutsDir)+string(os.PathSeparator)) {
		return fmt.Errorf("bad path")
	}
	if err := os.MkdirAll(filepath.Dir(full), 0o755); err != nil {
		return err
	}
	return os.WriteFile(full, body, 0o644)
}

func (h *DeviceHandler) audit(uid int64, action, target, detail string) {
	_, _ = h.db.Exec(`INSERT INTO audit_log (user_id, action, target, detail) VALUES (?,?,?,?)`,
		uid, action, target, detail)
}

// canSeeDevice returns true if the caller is allowed to read the device.
func (h *DeviceHandler) canSeeDevice(c *auth.Claims, deviceID int64, factoryID *int64) bool {
	if c == nil {
		return false
	}
	if c.Role == models.RoleSuperAdmin {
		return true
	}
	if factoryID == nil {
		return false
	}
	// Caller must belong to the device's factory.
	var n int
	if err := h.db.QueryRow(`SELECT COUNT(*) FROM user_factories WHERE user_id=? AND factory_id=?`,
		c.UserID, *factoryID).Scan(&n); err != nil || n == 0 {
		return false
	}
	if c.Role == models.RoleUser {
		// If a factory-level whitelist exists, the device must be in it.
		var wl int
		_ = h.db.QueryRow(`SELECT COUNT(*) FROM factory_visible_devices WHERE factory_id=?`, *factoryID).Scan(&wl)
		if wl > 0 {
			var ok int
			_ = h.db.QueryRow(`SELECT COUNT(*) FROM factory_visible_devices WHERE factory_id=? AND device_id=?`,
				*factoryID, deviceID).Scan(&ok)
			return ok > 0
		}
	}
	return true
}

// canMutateDevice returns true if the caller is allowed to modify/delete the device.
func (h *DeviceHandler) canMutateDevice(c *auth.Claims, deviceID int64) bool {
	if c == nil {
		return false
	}
	if c.Role == models.RoleSuperAdmin {
		return true
	}
	if c.Role != models.RoleAdmin {
		return false
	}
	var fid sql.NullInt64
	if err := h.db.QueryRow(`SELECT factory_id FROM devices WHERE id=?`, deviceID).Scan(&fid); err != nil {
		return false
	}
	if !fid.Valid {
		return false
	}
	var n int
	_ = h.db.QueryRow(`SELECT COUNT(*) FROM user_factories WHERE user_id=? AND factory_id=?`,
		c.UserID, fid.Int64).Scan(&n)
	return n > 0
}
