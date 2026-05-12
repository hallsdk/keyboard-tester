package config

import (
	"crypto/rand"
	"encoding/hex"
	"log"
	"os"
)

type Config struct {
	Listen         string
	DataDir        string
	JWTSecret      string
	SuperAdminUser string
	SuperAdminPass string
}

func Load() Config {
	c := Config{
		Listen:         getEnv("LISTEN_ADDR", ":3030"),
		DataDir:        getEnv("DATA_DIR", "./data"),
		JWTSecret:      os.Getenv("JWT_SECRET"),
		SuperAdminUser: os.Getenv("SUPER_ADMIN_USER"),
		SuperAdminPass: os.Getenv("SUPER_ADMIN_PASS"),
	}
	if c.JWTSecret == "" {
		b := make([]byte, 32)
		_, _ = rand.Read(b)
		c.JWTSecret = hex.EncodeToString(b)
		log.Println("WARNING: JWT_SECRET not set, using random ephemeral secret " +
			"(all tokens will become invalid on restart). Set JWT_SECRET in .env for production.")
	}
	return c
}

func getEnv(key, def string) string {
	if v := os.Getenv(key); v != "" {
		return v
	}
	return def
}
