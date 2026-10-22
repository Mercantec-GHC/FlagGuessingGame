
@echo off
cd /d "%~dp0"

docker build -t flag-guessing-game-api .
docker rm -f flag-guessing-game-api >nul 2>&1
docker run -d --env-file .env -p 5169:5169 --name flag-guessing-game-api flag-guessing-game-api