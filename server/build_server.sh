echo "Компиляция сервера"
mkdir -p build_server
cd build_server
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SERVER=ON \
    -DBUILD_CLIENT=OFF
make -j$(nproc)
if [ $? -eq 0 ]; then
    echo "Сервер скомпилирован!"
    echo "Запуск: ./build_server/auth_server"
else
    echo "Ошибка компиляции сервера"
    exit 1
fi
