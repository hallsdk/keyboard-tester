package models

import "time"

const (
	RoleUser       = "user"
	RoleAdmin      = "admin"
	RoleSuperAdmin = "super_admin"
)

type User struct {
	ID           int64      `json:"id"`
	Username     string     `json:"username"`
	Role         string     `json:"role"`
	CreatedAt    time.Time  `json:"created_at"`
	LastLogin    *time.Time `json:"last_login,omitempty"`
	PasswordHash string     `json:"-"`
}

type Device struct {
	ID          int64     `json:"id"`
	FactoryID   *int64    `json:"factory_id,omitempty"`
	VID         string    `json:"vid"`
	PID         string    `json:"pid"`
	Name        string    `json:"name"`
	Description string    `json:"description"`
	LayoutFile  string    `json:"layout_file"`
	CreatedBy   *int64    `json:"created_by,omitempty"`
	CreatedAt   time.Time `json:"created_at"`
	UpdatedAt   time.Time `json:"updated_at"`
}

type Factory struct {
	ID        int64     `json:"id"`
	Name      string    `json:"name"`
	CreatedAt time.Time `json:"created_at"`
}
