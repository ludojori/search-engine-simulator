#!/bin/bash

# Set variables
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
    sudo apt-get update && sudo apt-get install -y mysql-server || error "Failed to install MySQL server."
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
if ! sudo mysql -e "exit" &> /dev/null; then
    error "Unable to connect to MySQL. Ensure MySQL is running and accessible."
fi
log "MySQL connection successful."

# Load the database dump
log "Importing SQL dump file into MySQL..."
if sudo mysql < "$DUMP_FILE_FULL_PATH"; then
    log "✅ SQL dump imported successfully!"
else
    error "Failed to import SQL dump."
fi

# Create a new MySQL user and grant privileges
log "Creating MySQL user and granting privileges..."
if sudo mysql -e "CREATE USER IF NOT EXISTS 'mysql-user'@'localhost' IDENTIFIED BY 'mysql-password'; \
                  GRANT ALL PRIVILEGES ON search_engine_simulation.* TO 'mysql-user'@'localhost'; \
                  FLUSH PRIVILEGES;"; then
    log "✅ MySQL user 'mysql-user' created (or already exists) and privileges granted."
else
    error "Failed to create MySQL user or grant privileges."
fi

log "✅ MySQL setup is complete!"