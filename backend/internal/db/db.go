package db

import (
	"database/sql"
	"errors"
	"fmt"
	"log"

	_ "modernc.org/sqlite"

	"golang.org/x/crypto/bcrypt"
)

func Open(path string) (*sql.DB, error) {
	dsn := fmt.Sprintf("file:%s?_pragma=journal_mode(WAL)&_pragma=busy_timeout(5000)&_pragma=foreign_keys(1)", path)
	db, err := sql.Open("sqlite", dsn)
	if err != nil {
		return nil, err
	}
	db.SetMaxOpenConns(1) // SQLite write serialization is simpler this way
	if err := db.Ping(); err != nil {
		return nil, err
	}
	return db, nil
}

const schema = `
CREATE TABLE IF NOT EXISTS users (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    username      TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    role          TEXT NOT NULL CHECK(role IN ('user','admin','super_admin')),
    created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login    DATETIME
);

CREATE TABLE IF NOT EXISTS devices (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    vid         TEXT NOT NULL,
    pid         TEXT NOT NULL,
    name        TEXT NOT NULL,
    description TEXT DEFAULT '',
    layout_file TEXT NOT NULL,  -- filename inside data/layouts/
    created_by  INTEGER REFERENCES users(id) ON DELETE SET NULL,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(vid, pid)
);

CREATE TABLE IF NOT EXISTS audit_log (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id    INTEGER REFERENCES users(id) ON DELETE SET NULL,
    action     TEXT NOT NULL,
    target     TEXT,
    detail     TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
`

func Migrate(db *sql.DB) error {
	_, err := db.Exec(schema)
	return err
}

func BootstrapSuperAdmin(db *sql.DB, user, pass string) error {
	if user == "" || pass == "" {
		// Allow start without super admin only if at least one already exists.
		var n int
		if err := db.QueryRow(`SELECT COUNT(*) FROM users WHERE role='super_admin'`).Scan(&n); err != nil {
			return err
		}
		if n == 0 {
			log.Println("WARNING: no super_admin exists and SUPER_ADMIN_USER/PASS not set in env. " +
				"Set them in .env and restart to create one.")
		}
		return nil
	}
	// Upsert: if exists, ensure role=super_admin and update password.
	hash, err := bcrypt.GenerateFromPassword([]byte(pass), bcrypt.DefaultCost)
	if err != nil {
		return err
	}
	var id int64
	err = db.QueryRow(`SELECT id FROM users WHERE username = ?`, user).Scan(&id)
	if errors.Is(err, sql.ErrNoRows) {
		_, err = db.Exec(`INSERT INTO users (username, password_hash, role) VALUES (?,?, 'super_admin')`,
			user, string(hash))
		if err != nil {
			return err
		}
		log.Printf("created super_admin user: %s", user)
		return nil
	} else if err != nil {
		return err
	}
	_, err = db.Exec(`UPDATE users SET password_hash=?, role='super_admin' WHERE id=?`, string(hash), id)
	if err == nil {
		log.Printf("updated super_admin user: %s (password reset from env)", user)
	}
	return err
}
