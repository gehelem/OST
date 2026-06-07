# SSL/TLS Configuration for OST WebSocket Server

This document explains how to configure SSL/TLS encryption for the OST WebSocket server.

## Overview

By default, the OST WebSocket server runs in non-secure mode (ws://). You can enable SSL/TLS to use secure WebSocket connections (wss://).

## Quick Start

### 1. Generate Self-Signed Certificate (for testing)

```bash
./generate-ssl-cert.sh
```

This will create:
- `/etc/ost/server.crt` - SSL certificate
- `/etc/ost/server.key` - Private key

### 2. Start OST Server with SSL

```bash
ostserver --ssl Y --sslcert /etc/ost/server.crt --sslkey /etc/ost/server.key
```

## Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--ssl` | Enable SSL for WebSocket server | `N` |
| `--sslcert` | Path to SSL certificate file | `/etc/ost/server.crt` |
| `--sslkey` | Path to SSL private key file | `/etc/ost/server.key` |

## Usage Examples

### Non-secure mode (default)
```bash
ostserver
# WebSocket URL: ws://hostname:9624
```

### Secure mode with default certificate paths
```bash
ostserver --ssl Y
# WebSocket URL: wss://hostname:9624
```

### Secure mode with custom certificate paths
```bash
ostserver --ssl Y --sslcert /path/to/cert.pem --sslkey /path/to/key.pem
# WebSocket URL: wss://hostname:9624
```

## Production Deployment

For production environments, you should use certificates from a trusted Certificate Authority (CA) instead of self-signed certificates.

### Using Let's Encrypt

1. Install certbot:
```bash
sudo apt-get install certbot
```

2. Generate certificate:
```bash
sudo certbot certonly --standalone -d your-domain.com
```

3. Start OST with Let's Encrypt certificates:
```bash
ostserver --ssl Y \
  --sslcert /etc/letsencrypt/live/your-domain.com/fullchain.pem \
  --sslkey /etc/letsencrypt/live/your-domain.com/privkey.pem
```

### Certificate Format

The certificate and key files must be in PEM format. The implementation supports:
- **Certificate**: X.509 certificate in PEM format
- **Private Key**: RSA private key in PEM format
- **Protocol**: TLS 1.2 or later

## Frontend Configuration

When SSL is enabled on the server, you must update your frontend to connect using `wss://` instead of `ws://`:

```typescript
// Non-secure
const ws = new WebSocket('ws://hostname:9624');

// Secure
const ws = new WebSocket('wss://hostname:9624');
```

## Troubleshooting

### Server fails to start with SSL

Check the logs for error messages:
- "Cannot open SSL certificate file" - Certificate file path is incorrect or unreadable
- "Invalid SSL certificate" - Certificate file is corrupted or not in PEM format
- "Cannot open SSL private key file" - Key file path is incorrect or unreadable
- "Invalid SSL private key" - Key file is corrupted or not in PEM format

### Self-signed certificate warnings in browser

Self-signed certificates will trigger security warnings in web browsers. For testing:
1. Accept the security warning/exception in your browser
2. Or use the `--insecure` flag with tools like `curl` or `wscat`

For production, use a certificate from a trusted CA.

### Permission Issues

SSL certificate and key files require proper permissions:

```bash
sudo chown root:root /etc/ost/server.crt /etc/ost/server.key
sudo chmod 644 /etc/ost/server.crt
sudo chmod 600 /etc/ost/server.key
```

## Testing SSL Connection

### Using wscat

Install wscat:
```bash
npm install -g wscat
```

Test connection:
```bash
# Non-secure
wscat -c ws://localhost:9624

# Secure (with self-signed cert)
wscat -c wss://localhost:9624 --no-check
```

### Using openssl

Check certificate:
```bash
openssl x509 -in /etc/ost/server.crt -text -noout
```

Test TLS connection:
```bash
openssl s_client -connect localhost:9624 -servername localhost
```

## Security Considerations

1. **Never commit private keys to version control**
2. Use strong encryption (RSA 2048-bit minimum, or ECDSA)
3. Keep certificates up to date (monitor expiration)
4. Use TLS 1.2 or higher (TLS 1.0/1.1 are deprecated)
5. For production, always use certificates from trusted CAs
6. Protect private key files with appropriate permissions (600)

## Implementation Details

- The server uses Qt's `QWebSocketServer` in `SecureMode`
- SSL configuration is handled via `QSslConfiguration`
- Peer verification is disabled (`VerifyNone`) for self-signed certificates
- Protocol minimum is TLS 1.2 (`TlsV1_2OrLater`)
- The same port (9624) is used for both secure and non-secure modes
