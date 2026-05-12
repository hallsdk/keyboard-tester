# Multi-stage build — produces a tiny image with just the binary + web assets.
FROM golang:1.22-alpine AS build
WORKDIR /src
# Cache deps
COPY go.mod go.sum* ./
RUN go mod download || true
COPY . .
RUN CGO_ENABLED=0 go build -trimpath -ldflags="-s -w" -o /out/ktester ./cmd/server

FROM alpine:3.20
RUN apk add --no-cache ca-certificates tzdata && adduser -D -u 10001 app
WORKDIR /app
COPY --from=build /out/ktester /app/ktester
COPY --from=build /src/web /app/web
RUN mkdir -p /app/data && chown -R app:app /app
USER app
EXPOSE 8080
ENV LISTEN_ADDR=":8080" DATA_DIR="/app/data"
ENTRYPOINT ["/app/ktester"]
