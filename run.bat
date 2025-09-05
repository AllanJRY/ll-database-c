if exist ./employees.db (
    del employees.db
)

call ./build.bat

"./output/db.exe" -n -f ./employees.db -a "Allan,Rue dans Cournon,777"

"./output/db.exe" -f ./employees.db -l

"./output/db.exe" -f ./employees.db -a "Klara,Rue dans Cournon,872"
"./output/db.exe" -f ./employees.db -a "Yumi,Rue dans Cournon,0"
"./output/db.exe" -f ./employees.db -a "Luffy,Rue dans Cournon,0"

"./output/db.exe" -f ./employees.db -l

"./output/db.exe" -f ./employees.db -a "Cedric,Rue en Normandie,117"
"./output/db.exe" -f ./employees.db -a "Sean,Rue dans la Manche,1000"
"./output/db.exe" -f ./employees.db -a "Mimi,Rue dans Issoire,929"

"./output/db.exe" -f ./employees.db -l

"./output/db.exe" -f ./employees.db -a "Megan,Rue en Normandie,2"
"./output/db.exe" -f ./employees.db -a "Louis,Unknown,0"
"./output/db.exe" -f ./employees.db -a "Louis,Unknown,0"

"./output/db.exe" -f ./employees.db -l

"./output/db.exe" -f ./employees.db -d "Megan"

"./output/db.exe" -f ./employees.db -l

"./output/db.exe" -f ./employees.db -a "Test,Rue de test,19" -d "Louis"

"./output/db.exe" -f ./employees.db -l

