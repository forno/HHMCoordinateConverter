# build

```bash
mkdir build
cd build
cmake ..
make
```

# execute

```bash
./build/converter --config config/Cooking < cooking.csv
```

If you save coordinates in file:

```bash
./build/converter --config config/Cooking < cooking.csv > converted/1_162.csv
```
