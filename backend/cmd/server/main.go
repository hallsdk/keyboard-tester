package main

import (
	"context"
	"errors"
	"log"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"syscall"
	"time"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
	"github.com/go-chi/cors"

	"github.com/hallsdk/ktester-backend/internal/auth"
	"github.com/hallsdk/ktester-backend/internal/config"
	"github.com/hallsdk/ktester-backend/internal/db"
	"github.com/hallsdk/ktester-backend/internal/handlers"
)

func main() {
	cfg := config.Load()
	if err := os.MkdirAll(cfg.DataDir, 0o755); err != nil {
		log.Fatalf("create data dir: %v", err)
	}
	if err := os.MkdirAll(filepath.Join(cfg.DataDir, "layouts"), 0o755); err != nil {
		log.Fatalf("create layouts dir: %v", err)
	}

	database, err := db.Open(filepath.Join(cfg.DataDir, "ktester.db"))
	if err != nil {
		log.Fatalf("open db: %v", err)
	}
	defer database.Close()

	if err := db.Migrate(database); err != nil {
		log.Fatalf("migrate: %v", err)
	}
	if err := db.BootstrapSuperAdmin(database, cfg.SuperAdminUser, cfg.SuperAdminPass); err != nil {
		log.Fatalf("bootstrap super admin: %v", err)
	}

	jwtMgr := auth.NewJWTManager(cfg.JWTSecret, 24*time.Hour*30)

	authH := handlers.NewAuthHandler(database, jwtMgr)
	devH := handlers.NewDeviceHandler(database, filepath.Join(cfg.DataDir, "layouts"))
	userH := handlers.NewUserHandler(database)
	factH := handlers.NewFactoryHandler(database)

	r := chi.NewRouter()
	r.Use(middleware.RequestID)
	r.Use(middleware.RealIP)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)
	r.Use(middleware.Timeout(30 * time.Second))
	r.Use(cors.Handler(cors.Options{
		AllowedOrigins:   []string{"*"},
		AllowedMethods:   []string{"GET", "POST", "PUT", "DELETE", "OPTIONS"},
		AllowedHeaders:   []string{"Authorization", "Content-Type"},
		ExposedHeaders:   []string{"Content-Disposition"},
		AllowCredentials: false,
		MaxAge:           300,
	}))

	r.Route("/api", func(r chi.Router) {
		// Public endpoints
		r.Post("/auth/register", authH.Register)
		r.Post("/auth/login", authH.Login)

		// Authenticated endpoints
		r.Group(func(r chi.Router) {
			r.Use(auth.Middleware(jwtMgr))

			r.Get("/auth/me", authH.Me)
			r.Post("/auth/change-password", authH.ChangePassword)

			// Devices: list/get/download are for any authed user.
			r.Get("/devices", devH.List)
			r.Get("/devices/{vid}-{pid}", devH.Get)
			r.Get("/devices/{vid}-{pid}/layout", devH.DownloadLayout)

			// Admin-only mutations.
			r.Group(func(r chi.Router) {
				r.Use(auth.RequireRole("admin", "super_admin"))
				r.Post("/devices", devH.Create)
				r.Put("/devices/{id}", devH.Update)
				r.Put("/devices/{id}/layout", devH.UploadLayout)
				r.Delete("/devices/{id}", devH.Delete)
			})

			// User management:
			//   GET  /users                       — admin+ (scoped by shared factories)
			//   POST /users                       — super_admin only
			//   PUT  /users/{id}                  — super_admin only
			//   DEL  /users/{id}                  — super_admin only
			//   GET  /users/{id}/factories        — admin+ (super_admin assigns)
			//   PUT  /users/{id}/factories        — super_admin only
			//   GET  /users/{id}/devices          — admin+
			//   PUT  /users/{id}/devices          — admin+ (per-user whitelist)
			r.Group(func(r chi.Router) {
				r.Use(auth.RequireRole("admin", "super_admin"))
				r.Get("/users", userH.List)
				r.Get("/users/{id}/factories", factH.GetUserFactories)
				r.Get("/users/{id}/devices", factH.GetUserDevices)
				r.Put("/users/{id}/devices", factH.SetUserDevices)
				// Factories: list is visible to admin+ (their own); CRUD restricted below.
				r.Get("/factories", factH.List)
			})
			r.Group(func(r chi.Router) {
				r.Use(auth.RequireRole("super_admin"))
				r.Post("/users", userH.Create)
				r.Put("/users/{id}", userH.Update)
				r.Delete("/users/{id}", userH.Delete)
				r.Put("/users/{id}/factories", factH.SetUserFactories)
				r.Post("/factories", factH.Create)
				r.Put("/factories/{id}", factH.Update)
				r.Delete("/factories/{id}", factH.Delete)
			})
		})
	})

	// Static web UI.
	fs := http.FileServer(http.Dir("./web"))
	r.Handle("/*", fs)

	server := &http.Server{
		Addr:              cfg.Listen,
		Handler:           r,
		ReadHeaderTimeout: 10 * time.Second,
	}

	go func() {
		log.Printf("listening on %s", cfg.Listen)
		if err := server.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
			log.Fatalf("server: %v", err)
		}
	}()

	// Graceful shutdown.
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	log.Println("shutting down ...")
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	_ = server.Shutdown(ctx)
}
