package handlers

import (
	"database/sql"
	"encoding/json"
	"errors"
	"net/http"
	"strings"

	"golang.org/x/crypto/bcrypt"

	"github.com/hallsdk/ktester-backend/internal/auth"
)

type AuthHandler struct {
	db  *sql.DB
	jwt *auth.JWTManager
}

func NewAuthHandler(db *sql.DB, jm *auth.JWTManager) *AuthHandler {
	return &AuthHandler{db: db, jwt: jm}
}

type credentials struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

func (h *AuthHandler) Register(w http.ResponseWriter, r *http.Request) {
	var in struct {
		Username  string `json:"username"`
		Password  string `json:"password"`
		FactoryID int64  `json:"factory_id"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	in.Username = strings.TrimSpace(in.Username)
	if len(in.Username) < 3 || len(in.Username) > 32 {
		writeErr(w, 400, "username length 3-32")
		return
	}
	if len(in.Password) < 6 {
		writeErr(w, 400, "password too short (min 6)")
		return
	}
	if in.FactoryID == 0 {
		writeErr(w, 400, "factory_id required")
		return
	}
	// Verify factory exists.
	var fn string
	if err := h.db.QueryRow(`SELECT name FROM factories WHERE id=?`, in.FactoryID).Scan(&fn); errors.Is(err, sql.ErrNoRows) {
		writeErr(w, 400, "factory not found")
		return
	} else if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	hash, err := bcrypt.GenerateFromPassword([]byte(in.Password), bcrypt.DefaultCost)
	if err != nil {
		writeErr(w, 500, "hash error")
		return
	}
	_, err = h.db.Exec(
		`INSERT INTO users (username, password_hash, role, status, pending_factory_id) VALUES (?,?,'user','pending',?)`,
		in.Username, string(hash), in.FactoryID)
	if err != nil {
		if strings.Contains(err.Error(), "UNIQUE") {
			writeErr(w, 409, "username already exists")
			return
		}
		writeErr(w, 500, "db error")
		return
	}
	writeJSON(w, 201, map[string]any{
		"status":       "pending",
		"message":      "\u6ce8\u518c\u6210\u529f\uff0c\u8bf7\u7b49\u5f85\u7ba1\u7406\u5458\u5ba1\u6838",
		"factory_name": fn,
	})
}

func (h *AuthHandler) Login(w http.ResponseWriter, r *http.Request) {
	var in credentials
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	var (
		id       int64
		hash     string
		role     string
		username string
		status   string
	)
	err := h.db.QueryRow(
		`SELECT id, username, password_hash, role, status FROM users WHERE username = ?`,
		strings.TrimSpace(in.Username),
	).Scan(&id, &username, &hash, &role, &status)
	if errors.Is(err, sql.ErrNoRows) {
		writeErr(w, 401, "invalid credentials")
		return
	} else if err != nil {
		writeErr(w, 500, "db error")
		return
	}
	if bcrypt.CompareHashAndPassword([]byte(hash), []byte(in.Password)) != nil {
		writeErr(w, 401, "invalid credentials")
		return
	}
	if status == "pending" {
		writeErr(w, 403, "\u8d26\u53f7\u5f85\u5ba1\u6838\uff0c\u8bf7\u8054\u7cfb\u7ba1\u7406\u5458")
		return
	}
	_, _ = h.db.Exec(`UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = ?`, id)
	token, _, _ := h.jwt.Issue(id, username, role)
	writeJSON(w, 200, map[string]any{
		"token":    token,
		"username": username,
		"role":     role,
	})
}

func (h *AuthHandler) Me(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	writeJSON(w, 200, map[string]any{
		"id":       c.UserID,
		"username": c.Username,
		"role":     c.Role,
	})
}

func (h *AuthHandler) ChangePassword(w http.ResponseWriter, r *http.Request) {
	c := auth.ClaimsFrom(r)
	var in struct {
		OldPassword string `json:"old_password"`
		NewPassword string `json:"new_password"`
	}
	if err := json.NewDecoder(r.Body).Decode(&in); err != nil {
		writeErr(w, 400, "invalid json")
		return
	}
	if len(in.NewPassword) < 6 {
		writeErr(w, 400, "new password too short")
		return
	}
	var hash string
	if err := h.db.QueryRow(`SELECT password_hash FROM users WHERE id=?`, c.UserID).Scan(&hash); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	if bcrypt.CompareHashAndPassword([]byte(hash), []byte(in.OldPassword)) != nil {
		writeErr(w, 401, "old password incorrect")
		return
	}
	nh, err := bcrypt.GenerateFromPassword([]byte(in.NewPassword), bcrypt.DefaultCost)
	if err != nil {
		writeErr(w, 500, "hash error")
		return
	}
	if _, err := h.db.Exec(`UPDATE users SET password_hash=? WHERE id=?`, string(nh), c.UserID); err != nil {
		writeErr(w, 500, "db error")
		return
	}
	writeJSON(w, 200, map[string]string{"status": "ok"})
}
