# HMSSQL

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-brightgreen.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![GitHub issues](https://img.shields.io/github/issues-raw/dedzsinator/hmssql)](https://github.com/dedzsinator/hmssql/issues)
[![GitHub stars](https://img.shields.io/github/stars/dedzsinator/hmssql?style=social)](https://github.com/dedzsinator/hmssql/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/dedzsinator/hmssql?style=social)](https://github.com/dedzsinator/hmssql/network)
[![Last Commit](https://img.shields.io/github/last-commit/dedzsinator/hmssql/main)](https://github.com/dedzsinator/hmssql/commits/main)
[![Documentation Status](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://github.com/dedzsinator/hmssql/wiki)
[![Test Coverage](https://img.shields.io/badge/coverage-80%25-yellowgreen.svg)](https://github.com/dedzsinator/hmssql/actions)
[![Language](https://img.shields.io/github/languages/top/dedzsinator/HMSSQL)](https://github.com/dedzsinator/HMSSQL)
[![Release](https://img.shields.io/github/v/release/dedzsinator/HMSSQL)](https://github.com/dedzsinator/HMSSQL/releases)
[![Lines of Code](https://img.shields.io/github/languages/code-size/dedzsinator/hmssql)](https://github.com/dedzsinator/hmssql)
[![Lines of Code](https://sloc.xyz/github/dedzsinator/hmssql?category=code)](https://github.com/dedzsinator/hmssql)

Oktatási célú relációs adatbázis-kezelő rendszer. A HMSSQL egy egyszerű SQL motorral és interaktív shell-lel rendelkezik.

## 🚀 Gyors kezdés

## Projekt klónozása

```bash
git clone https://github.com/dedzsinator/hmssql.git
cd hmssql
```

### Fordítás és telepítés

```bash
sudo ./preinstall.sh
mkdir build && cd build
cmake ..
make -j4
```

megjegyzés, hogy a '-j4' a szálak számát jelenti, amelyeket a fordítás során használni fog. A számot a processzor magjainak számára kell beállítani ahogy kívánja.

## Miket tud?

### 🛠️ Parancsok

- \dt - Táblák listázása
- \di - Indexek listázása
- \dc - Adatbázisok és tábláik listázása
- \save - Adatbázis állapot mentése
- \checkpoint - Manuális checkpoint készítése
- \help - Súgó megjelenítése

### 🔍 Debug vs Production mód

#### Debug mód

```bash
cmake -DISDEBUG=ON ..
```

- AddressSanitizer engedélyezve
- Debug szimbólumok
- Tesztek és mock objektumok elérhetők

#### Production mód

```bash
cmake -DISDEBUG=OFF ..
```

- Optimalizált teljesítmény
- Mock és teszt kód kizárva
- Minimális méretű bináris

### 🌐 Web Felület Indítása

#### Előfeltételek Telepítése

```bash
# Navigálás a web könyvtárba
cd tools/web

# Node.js függőségek telepítése
npm install express child_process path
```

#### Package.json Konfiguráció

```json
{
  "name": "hmssql-web",
  "version": "2.0.0",
  "scripts": {
    "start": "node server.js"
  },
  "dependencies": {
    "express": "^4.18.2",
    "child_process": "^1.0.2"
  }
}
```

#### HMSSQL Daemon Fordítása

```bash
# Projekt gyökérkönyvtárában
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Web Szerver Indítása

1. Tailwind CSS Figyelő Indítása (első terminál):

```bash
cd tools/web
npm run build:css
```

2. Web Szerver Indítása (második terminál):

```bash
cd tools/web
node server.js
```

A web felület ezután elérhető a következő címen: [http://localhost:3000]

#### Web Felület Hibái

- Ellenőrizze, hogy a daemon sikeresen lefordult-e
- Győződjön meg róla, hogy a 3000-es port szabad
- Ellenőrizze, hogy minden npm csomag telepítve van-e
- Ellenőrizze, hogy a daemon fut-e a 8080-as porton

### 🔄 Daemon Kezelése

```bash
# Jogosultság beállítása a szkripthez
sudo chmod +x tools/scripts/manage_daemon.sh

# Felhasználó és csoport létrehozása
sudo useradd -r -s /bin/false hmssql
sudo groupadd hmssql

# Daemon telepítése és indítása
sudo ./tools/scripts/manage_daemon.sh

# Daemon állapotának ellenőrzése
sudo systemctl status hmssql

# Daemon vezérlése
sudo systemctl start hmssql    # Indítás
sudo systemctl stop hmssql     # Leállítás
sudo systemctl restart hmssql  # Újraindítás
sudo systemctl enable hmssql   # Automatikus indítás beállítása

# Naplók megtekintése
journalctl -u hmssql -f        # Valós idejű napló követése
journalctl -u hmssql -n 50     # Utolsó 50 naplóbejegyzés
```

#### Daemon Hibái

- Ha a daemon nem indul: `journalctl -u hmssql -n 50`
- Jogosultságok ellenőrzése: `ls -l /usr/local/bin/hmssql_daemon`
- Port foglaltság ellenőrzése: `sudo lsof -i :8080`
- Folyamat ellenőrzése: `ps aux | grep hmssql`

```sql
-- Adatbázis létrehozása
CREATE DATABASE pelda;
USE pelda;

-- Tábla létrehozása
CREATE TABLE felhasznalok (
    id INT PRIMARY KEY,
    nev VARCHAR(50),
    email VARCHAR(100)
);

-- Adat beszúrása
INSERT INTO felhasznalok VALUES (1, 'Kiss János', 'kiss.janos@pelda.hu');

-- Lekérdezés
SELECT * FROM felhasznalok;
```

## 🤝 Kontribúció otletek fontossagi sorrendben

- [ ] Tesztek irása !!!
- [ ] Windwos támogatás
- [ ] Desktop frontend
- [ ] Docker támogatás
- [ ] Web felület fejlesztése
- [ ] Dokumentáció frissítése
- [ ] SQL Engine optimalizálása
- [ ] Saját SQL Parser
- [ ] XML (or json) metadata (instead of config.h)
