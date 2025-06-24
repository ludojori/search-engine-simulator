DROP DATABASE IF EXISTS search_engine_simulation;
CREATE DATABASE search_engine_simulation;
USE search_engine_simulation;

CREATE TABLE pairs (
    id INT NOT NULL AUTO_INCREMENT,
    origin VARCHAR(3) NOT NULL,
    destination VARCHAR(3) NOT NULL,
    type BOOLEAN NOT NULL DEFAULT 0, -- 0 - one way, 1 - roundtrip
    f_carrier VARCHAR(2),
    PRIMARY KEY (id),
    UNIQUE (origin, destination)
);

INSERT INTO pairs (origin, destination, type, f_carrier)
VALUES
("BLA", "MUC", 0, "FF"),
("SOF", "LON", 1, "FB"),
("LON", "FRA", 1, "BA");

CREATE TABLE user_types (
    id INT NOT NULL AUTO_INCREMENT,
    type VARCHAR(8) NOT NULL,
    PRIMARY KEY (id),
    UNIQUE (type)
);

INSERT INTO user_types (type)
VALUES
("Internal"),
("External"),
("Manager");

CREATE TABLE users (
    id INT NOT NULL AUTO_INCREMENT,
    name VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    type_id INT NOT NULL,
    PRIMARY KEY (id),
    FOREIGN KEY (type_id) REFERENCES user_types(id),
    UNIQUE (name)
);

INSERT INTO users (name, password, type_id)
VALUES
("AirBaltic", "password1", 1),
("BulgariaAir", "password2", 1),
("Secretuser205", "password3", 2),
("SuperAdmin1", "4strong_Password4", 3);

CREATE TABLE flights (
    id INT NOT NULL AUTO_INCREMENT,
    pair_id INT NOT NULL,
    dep_datetime DATETIME NOT NULL,
    arr_datetime DATETIME NOT NULL,
    price FLOAT NOT NULL,
    currency VARCHAR(3) NOT NULL,
    cabin VARCHAR(1) NOT NULL,
    PRIMARY KEY (id),
    FOREIGN KEY (pair_id) REFERENCES pairs(id)
);

-- Table `flights` will be populated by the application.