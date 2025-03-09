#!/bin/bash

# Set variables
MYSQL_USER="root"
MYSQL_PASS="password"
DUMP_FILE="/sql/create-tables.sql"

# Helper function to display messages
log() {
    echo "[INFO] $1"
}

error() {
    echo "[ERROR] $1"
    exit 1
}

# Check if the MySQL client is installed
if ! command -v mysql &> /dev/null; then
    error "MySQL client (mysql) is not installed. Please install it and try again."
fi
log "MySQL client is installed."

LIB_MYSQL_CLIENT_DEV_INSTALLED=false
LIB_MYSQL_SERVER_INSTALLED=false

# Check if MySQL development libraries are installed
if ! dpkg -l | grep -q "libmysqlclient-dev"; then
    log "libmysqlclient-dev libraries not found. Trying to install..."
    sudo apt-get update && sudo apt-get install -y libmysqlclient-dev || error "Failed to install MySQL dev libraries."
fi

LIB_MYSQL_CLIENT_DEV_INSTALLED=true

if ! dpkg -l | grep -q "mysql-server"; then
    log "mysql-server libraries not found. Trying to install..."
    sudo apt-get update && sudo apt-get install -y libmysqlclient-dev || error "Failed to install MySQL dev libraries."
fi

LIB_MYSQL_SERVER_INSTALLED=true

if [[ "$LIB_MYSQL_CLIENT_DEV_INSTALLED" == false || "$LIB_MYSQL_SERVER_INSTALLED" == false ]]; then
    error "MySQL libraries missing. Install them manually (e.g., libmysqlclient-dev, mysql-server)."
fi

# Start MySQL daemon (mysqld)
sudo service mysql start || error "Failed to start MySQL service."

# Check if the MySQL daemon (mysqld) is running
if ! pgrep -x "mysqld" > /dev/null; then
    error "mysqld process is not running. Start MySQL service first (sudo may be required)."
fi
log "mysqld process is running."

# Ensure the dump file exists in the current directory
DUMP_FILE_FULL_PATH="$(pwd)$DUMP_FILE"
if [[ ! -f "$DUMP_FILE_FULL_PATH" ]]; then
    error "SQL dump file '$DUMP_FILE_FULL_PATH' not found."
fi
log "SQL dump file found: $DUMP_FILE_FULL_PATH"

# Test MySQL connection
log "Checking MySQL connection..."
if ! mysql -u "$MYSQL_USER" -p"$MYSQL_PASS" -e "exit" &> /dev/null; then
    error "Unable to connect to MySQL. Check login credentials."
fi
log "MySQL connection successful."

# Load the database dump
log "Importing SQL dump file into MySQL..."
if mysql -u "$MYSQL_USER" -p"$MYSQL_PASS" < "$DUMP_FILE_FULL_PATH"; then
    log "✅ MySQL setup is complete!"
fi
