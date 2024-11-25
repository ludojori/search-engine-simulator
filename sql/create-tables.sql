CREATE TABLE IF NOT EXISTS
    pairs (
        id INT NOT NULL AUTO_INCREMENT,
        origin VARCHAR(3) NOT NULL,
        destination VARCHAR(3) NOT NULL,
        is_one_way TINYINT NOT NULL DEFAULT 0,
        is_roundtrip TINYINT NOT NULL DEFAULT 1,
        f_carrier VARCHAR(2),
        m_carrier VARCHAR(2),
        o_carrier VARCHAR(2),
        PRIMARY KEY (id),
        UNIQUE (origin, destination)
    );

INSERT INTO pairs
VALUES
(0, "BLA", "MUC", 0, 1, "FF", "JF", "X3"),
(1, "SOF", "LON", 1, 0, "FB", "FB", "FB"),
(2, "LON", "FRA", 1, 0, "BA", "LS", "BA");

CREATE TABLE IF NOT EXISTS
    users (
        id INT NOT NULL AUTO_INCREMENT,
        name VARCHAR(255) NOT NULL,
        password VARCHAR(255) NOT NULL,
        type_id INT NOT NULL,
        PRIMARY KEY (id),
        UNIQUE (name)
    );

INSERT INTO users
VALUES
(0, "AirBaltic", "password1", 0),
(1, "BulgariaAir", "password2", 0),
(2, "Secretuser205", "password3", 1),
(3, "SuperAdmin1", "4strong_Password4", 2); 

CREATE TABLE IF NOT EXISTS
    user_types (
        id INT NOT NULL AUTO_INCREMENT,
        type VARCHAR(8) NOT NULL,
        PRIMARY KEY (id),
        UNIQUE (type)
    );

INSERT INTO user_types
VALUES
(0, "Internal"),
(1, "External"),
(2, "Manager");
