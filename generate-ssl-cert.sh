#!/bin/bash
#
# Generate self-signed SSL certificate for OST WebSocket server
# Usage: ./generate-ssl-cert.sh [output_directory]
#

OUTPUT_DIR="${1:-/etc/ost}"
CERT_FILE="$OUTPUT_DIR/server.crt"
KEY_FILE="$OUTPUT_DIR/server.key"
DAYS=365

echo "Generating self-signed SSL certificate for OST server..."
echo "Output directory: $OUTPUT_DIR"

# Create output directory if it doesn't exist
if [ ! -d "$OUTPUT_DIR" ]; then
    echo "Creating directory: $OUTPUT_DIR"
    sudo mkdir -p "$OUTPUT_DIR"
fi

# Generate private key and certificate
sudo openssl req -x509 -nodes -days $DAYS -newkey rsa:2048 \
    -keyout "$KEY_FILE" \
    -out "$CERT_FILE" \
    -subj "/C=FR/ST=France/L=Paris/O=OST/OU=Observatory/CN=localhost"

if [ $? -eq 0 ]; then
    echo ""
    echo "SSL certificate generated successfully!"
    echo "Certificate: $CERT_FILE"
    echo "Private key: $KEY_FILE"
    echo ""
    echo "To start OST server with SSL:"
    echo "  ostserver --ssl Y --sslcert $CERT_FILE --sslkey $KEY_FILE"
    echo ""
    echo "Note: This is a self-signed certificate for testing purposes."
    echo "For production, use a certificate from a trusted CA."
else
    echo "ERROR: Failed to generate SSL certificate"
    exit 1
fi
