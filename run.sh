#!/bin/bash

if [ -e employees.db ] ; then
    rm employees.db
fi

bash build.sh

"output/db" -n -f ./employees.db -a "Allan,Rue dans Cournon,777"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -a "Klara,Rue dans Cournon,872"
"output/db" -f ./employees.db -a "Yumi,Rue dans Cournon,0"
"output/db" -f ./employees.db -a "Luffy,Rue dans Cournon,0"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -a "Cedric,Rue en Normandie,117"
"output/db" -f ./employees.db -a "Sean,Rue dans la Manche,1000"
"output/db" -f ./employees.db -a "Mimi,Rue dans Issoire,929"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -a "Megan,Rue en Normandie,2"

"output/db" -f ./employees.db -a "Louis,Unknown,0"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -a "Louis,Unknown,0" -u "Megan:To Delete,10"

"output/db" -f ./employees.db -u "Louis:Unknown Updated,10"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -d "Megan"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -u "Test:Quelque part,100"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -a "Test,Rue de test,19" -d "Louis"

"output/db" -f ./employees.db -l

"output/db" -f ./employees.db -u "Klara:Cournon,200"

"output/db" -f ./employees.db -l
