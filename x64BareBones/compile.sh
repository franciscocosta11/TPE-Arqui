docker start TPE

# Limpiar todo
docker exec -it TPE make clean -C /root/Toolchain
docker exec -it TPE make clean -C /root/

# Construir Toolchain
docker exec -it TPE make all -C /root/Toolchain

# Construir todo el proyecto
docker exec -it TPE make all -C /root/

sudo chown -R $USER:$USER .
sudo chmod -R 755 .

# Verificar que se crearon los archivos necesarios
ls -la Image/x64BareBonesImage.*