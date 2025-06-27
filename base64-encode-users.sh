#!/bin/bash

# Variables
DB_USER="mysql-user"
DB_PASS="mysql-password"
DB_NAME="search_engine_simulation"

# Query users table and process each row
mysql -u"$DB_USER" -p"$DB_PASS" -D"$DB_NAME" -Bse "SELECT name, password FROM users;" | while IFS=$'\t' read -r username password
do
    raw="${username}:${password}"
    encoded=$(echo -n "$raw" | base64)
    echo "$username => $encoded"
done
