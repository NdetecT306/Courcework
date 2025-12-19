#!/bin/bash

echo "Запуск защищенного клиента"

# Проверяем наличие SSL сертификата
if [ ! -f "server.crt" ]; then
    echo "SSL сертификат сервера не найден. Копируем из ssl_certs..."
    cp ssl_certs/server.crt .
fi

echo "Компиляция клиента"
mkdir -p build_client
cd build_client

cmake .. -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_CLIENT=ON \
    -DBUILD_SERVER=OFF

make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Защищенный клиент скомпилирован!"
    echo "Используется TLS 1.3 для шифрования соединения"
    echo ""
    echo "Файл: $(pwd)/auth_client"
    echo "Сертификат: $(pwd)/../server.crt"
    echo ""
    echo "Запуск: ./auth_client"
    echo ""
    echo "Для проверки TLS соединения:"
    echo "1. Запустите сервер"
    echo "2. Выполните: openssl s_client -connect localhost:8080 -tls1_3 -CAfile server.crt"
else
    echo "Ошибка компиляции клиента"
    exit 1
fi
