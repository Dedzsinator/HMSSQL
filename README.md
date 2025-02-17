# HMSSQL

[![Build Status](https://img.shields.io/github/actions/workflow/status/dedzsinator/hmssql/build.yml?branch=main)](https://github.com/dedzsinator/hmssql/actions/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-brightgreen.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![GitHub issues](https://img.shields.io/github/issues-raw/dedzsinator/hmssql)](https://github.com/dedzsinator/hmssql/issues)
[![GitHub stars](https://img.shields.io/github/stars/dedzsinator/hmssql?style=social)](https://github.com/dedzsinator/hmssql/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/dedzsinator/hmssql?style=social)](https://github.com/dedzsinator/hmssql/network)
[![Last Commit](https://img.shields.io/github/last-commit/dedzsinator/hmssql/main)](https://github.com/dedzsinator/hmssql/commits/main)
[![Documentation Status](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://github.com/dedzsinator/hmssql/wiki)
[![Test Coverage](https://img.shields.io/badge/coverage-80%25-yellowgreen.svg)](https://github.com/dedzsinator/hmssql/actions)
[![Language](https://img.shields.io/github/languages/top/dedzsinator/hmssql)](https://github.com/dedzsinator/hmssql)
[![Release](https://img.shields.io/github/v/release/dedzsinator/hmssql?include_prereleases)](https://github.com/dedzsinator/hmssql/releases)
[![Lines of Code](https://img.shields.io/tokei/lines/github/dedzsinator/hmssql)](https://github.com/dedzsinator/hmssql)

Oktatási célú relációs adatbázis-kezelő rendszer. A HMSSQL egy egyszerű SQL motorral és interaktív shell-lel rendelkezik.

## 🚀 Gyors kezdés

### Fordítás és telepítés

```bash
sudo ./build_support/packages.sh
```

## Projekt klónozása

```bash
git clone https://github.com/dedzsinator/hmssql.git
cd hmssql
```

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
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

- AddressSanitizer engedélyezve
- Debug szimbólumok
- Tesztek és mock objektumok elérhetők

#### Production mód

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DHMSSQL_BUILD_PROD=ON ..
```

-Optimalizált teljesítmény
-Mock és teszt kód kizárva
-Minimális méretű bináris

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
