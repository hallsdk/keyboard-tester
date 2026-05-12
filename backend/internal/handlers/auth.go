package handlers

import (
	"database/sql"
	"encoding/json"
	"errors"
	"net/http"
	"strings"

	"golang.org/x/crypto/bcrypt"

	"github.com/hallsdk/ktester-backend/internal/auth"
	"github.com/hallsdk/ktester-backend/internal/models"
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
	var in credentials
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
	hash, err := bcrypt.GenerateFromPassword([]byte(in.Password), bcrypt.DefaultCost)
	if err != nil {
		writeErr(w, 500, "hash error")
		return
	}
	res, err := h.db.Exec(
		`INSERT INTO users (username, password_hash, role) VALUES (?,?, 'user')`,
		in.Username, string(hash))
	if err != nil {
		if strings.Contains(err.Error(), "UNIQUE") {
			writeErr(w, 409, "username already exists")
			return
		}
		writeErr(w, 500, "db error")
		return
	}
	id, _ := res.LastInsertId()
	token, _, _ := h.jwt.Issue(id, in.Username, models.RoleUser)
	writeJSON(w, 201, map[string]any{
		"token":    token,
		"username": in.Username,
		"role":     models.RoleUser,
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
	)
	err := h.db.QueryRow(
		`SELECT id, username, password_hash, role FROM users WHERE username = ?`,
		strings.TrimSpace(in.Username),
	).Scan(&id, &username, &hash, &role)
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
