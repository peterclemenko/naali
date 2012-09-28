DROP DATABASE IF EXISTS avatar;
CREATE DATABASE avatar;
USE avatar;

CREATE TABLE user(
    userId INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
    userName VARCHAR(30) NOT NULL ,
    userEmail VARCHAR(50) NOT NULL,
    userPassword VARCHAR(256) NOT NULL,
    UNIQUE (userName),
    UNIQUE (userEmail)
)ENGINE = InnoDB;

/*admin::admin*/
INSERT INTO user(userName, userEmail, userPassword) VALUES('admin', 'admin@host.com', '$1$FSL.ycce$TYNM1ZN4MY/vZPNi42Zoj0');
INSERT INTO user(userName, userEmail, userPassword) VALUES('test1', 'test1@host.com', '$1$FSL.ycce$TYNM1ZN4MY/vZPNi42Zoj0');
INSERT INTO user(userName, userEmail, userPassword) VALUES('test2', 'test2@host.com', '$1$FSL.ycce$TYNM1ZN4MY/vZPNi42Zoj0');


CREATE TABLE avatar(
    avatarId INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
    avatarName VARCHAR(30) NOT NULL,
    avatarScale DECIMAL(10,3) NOT NULL,
    avatarFile VARCHAR(100) NOT NULL,
    UNIQUE (avatarName),
    UNIQUE (avatarFile)
)ENGINE = InnoDB;

INSERT INTO avatar(avatarName, avatarScale, avatarFile) VALUES('first', '0.1', 'avatar1.dae');
INSERT INTO avatar(avatarName, avatarScale, avatarFile) VALUES('second', '0.1', 'avatar2.dae');
INSERT INTO avatar(avatarName, avatarScale, avatarFile) VALUES('third', '0.1', 'avatar3.dae');
INSERT INTO avatar(avatarName, avatarScale, avatarFile) VALUES('mandun70', '0.8', 'Mandun70.dae');
INSERT INTO avatar(avatarName, avatarScale, avatarFile) VALUES('swat', '0.02', 'swat.dae');


CREATE TABLE useravatar(
    userId INT NOT NULL,
    avatarId INT NOT NULL,
    FOREIGN KEY (userId) REFERENCES user(userId),
    FOREIGN KEY (avatarId) REFERENCES avatar(avatarId)
)ENGINE = InnoDB;

INSERT INTO useravatar(userId, avatarId) VALUES('1', '2');
INSERT INTO useravatar(userId, avatarId) VALUES('2', '1');
INSERT INTO useravatar(userId, avatarId) VALUES('3', '3');

