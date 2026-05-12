package db

import (
	"database/sql"
	"errors"
	"fmt"
	"log"
	"strings"

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

CREATE TABLE IF NOT EXISTS factories (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    name       TEXT UNIQUE NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS user_factories (
    user_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    factory_id INTEGER NOT NULL REFERENCES factories(id) ON DELETE CASCADE,
    PRIMARY KEY (user_id, factory_id)
);

CREATE TABLE IF NOT EXISTS devices (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    factory_id  INTEGER REFERENCES factories(id) ON DELETE SET NULL,
    vid         TEXT NOT NULL,
    pid         TEXT NOT NULL,
    name        TEXT NOT NULL,
    description TEXT DEFAULT '',
    layout_file TEXT NOT NULL,
    created_by  INTEGER REFERENCES users(id) ON DELETE SET NULL,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(vid, pid, factory_id)
);

CREATE TABLE IF NOT EXISTS user_devices (
    user_id   INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    device_id INTEGER NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    PRIMARY KEY (user_id, device_id)
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
	if _, err := db.Exec(schema); err != nil {
		return err
	}
	// Older databases predate the factory model. Backfill in-place.
	if err := migrateAddFactoryIDOnDevices(db); err != nil {
		return err
	}
	if err := ensureDefaultFactory(db); err != nil {
		return err
	}
	if err := migrateUniqueVidPidPerFactory(db); err != nil {
		return err
	}
	return nil
}

// Add devices.factory_id if the column is missing on an older DB.
func migrateAddFactoryIDOnDevices(db *sql.DB) error {
	rows, err := db.Query(`PRAGMA table_info(devices)`)
	if err != nil {
		return err
	}
	defer rows.Close()
	has := false
	for rows.Next() {
		var (
			cid     int
			name    string
			ctype   string
			notnull int
			dflt    sql.NullString
			pk      int
		)
		if err := rows.Scan(&cid, &name, &ctype, &notnull, &dflt, &pk); err != nil {
			return err
		}
		if name == "factory_id" {
			has = true
		}
	}
	if !has {
		if _, err := db.Exec(`ALTER TABLE devices ADD COLUMN factory_id INTEGER REFERENCES factories(id) ON DELETE SET NULL`); err != nil {
			return err
		}
		log.Println("migration: added devices.factory_id column")
	}
	return nil
}

// Change UNIQUE(vid,pid) → UNIQUE(vid,pid,factory_id) so the same VID/PID can
// exist in multiple factories. SQLite requires a full table rebuild to change a
// constraint.
func migrateUniqueVidPidPerFactory(db *sql.DB) error {
	var tableSql string
	if err := db.QueryRow(`SELECT sql FROM sqlite_master WHERE type='table' AND name='devices'`).Scan(&tableSql); err != nil {
		return err
	}
	// Already migrated if the constraint includes factory_id.
	if strings.Contains(tableSql, "UNIQUE(vid, pid, factory_id)") || strings.Contains(tableSql, "UNIQUE(vid,pid,factory_id)") {
		return nil
	}
	// Only proceed if the old narrow constraint is present.
	if !strings.Contains(tableSql, "UNIQUE(vid, pid)") && !strings.Contains(tableSql, "UNIQUE(vid,pid)") {
		return nil
	}
	// Disable FK checks for the duration — we're just reshaping the table, data stays valid.
	if _, err := db.Exec(`PRAGMA foreign_keys=OFF`); err != nil {
		return err
	}
	defer db.Exec(`PRAGMA foreign_keys=ON`)

	// Clean up any leftover from a previous failed attempt.
	if _, err := db.Exec(`DROP TABLE IF EXISTS devices_new`); err != nil {
		return err
	}
	_, err := db.Exec(`
		CREATE TABLE devices_new (
			id          INTEGER PRIMARY KEY AUTOINCREMENT,
			factory_id  INTEGER REFERENCES factories(id) ON DELETE SET NULL,
			vid         TEXT NOT NULL,
			pid         TEXT NOT NULL,
			name        TEXT NOT NULL,
			description TEXT DEFAULT '',
			layout_file TEXT NOT NULL,
			created_by  INTEGER REFERENCES users(id) ON DELETE SET NULL,
			created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
			updated_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
			UNIQUE(vid, pid, factory_id)
		)`)
	if err != nil {
		return fmt.Errorf("migration create devices_new: %w", err)
	}
	if _, err := db.Exec(`INSERT INTO devices_new
		(id, vid, pid, name, description, layout_file, created_by, created_at, updated_at, factory_id)
		SELECT id, vid, pid, name, description, layout_file, created_by, created_at, updated_at, factory_id
		FROM devices`); err != nil {
		return fmt.Errorf("migration copy devices: %w", err)
	}
	if _, err := db.Exec(`DROP TABLE devices`); err != nil {
		return fmt.Errorf("migration drop devices: %w", err)
	}
	if _, err := db.Exec(`ALTER TABLE devices_new RENAME TO devices`); err != nil {
		return fmt.Errorf("migration rename devices: %w", err)
	}
	log.Println("migration: changed devices unique constraint to (vid, pid, factory_id)")
	return nil
}

// Ensure a default factory exists and back-fill existing data.
func ensureDefaultFactory(db *sql.DB) error {
	var n int
	if err := db.QueryRow(`SELECT COUNT(*) FROM factories`).Scan(&n); err != nil {
		return err
	}
	if n == 0 {
		if _, err := db.Exec(`INSERT INTO factories (id, name) VALUES (1, '默认工厂')`); err != nil {
			return err
		}
		log.Println("migration: created default factory id=1")
	}
	// Assign factory-less devices to default.
	if _, err := db.Exec(`UPDATE devices SET factory_id = 1 WHERE factory_id IS NULL`); err != nil {
		return err
	}
	// Assign factory-less non-super users to default.
	if _, err := db.Exec(`
		INSERT OR IGNORE INTO user_factories (user_id, factory_id)
		SELECT u.id, 1 FROM users u
		WHERE u.role != 'super_admin'
		  AND NOT EXISTS (SELECT 1 FROM user_factories f WHERE f.user_id = u.id)
	`); err != nil {
		return err
	}
	return nil
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
