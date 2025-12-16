echo "Компиляция клиента"
# Создаем директорию для сборки
mkdir -p build_client
cd build_client
# Очищаем предыдущую сборку
rm -rf *
# Генерируем проект 
cmake .. -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_CLIENT=ON \
    -DBUILD_SERVER=OFF
make -j$(nproc)
if [ $? -eq 0 ]; then
    echo "Клиент скомпилирован!"
    echo "Файл: $(pwd)/auth_client"
    echo "Запуск: ./auth_client"
    echo "Если будут ошибки, запустить с отладкой:"
    echo "gdb ./auth_client"
else
    echo "Ошибка компиляции клиента"
    exit 1
fi
