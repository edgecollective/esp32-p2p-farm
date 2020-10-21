#!/bin/bash

# vale 3

curl --location --request PUT 'http://64.227.0.108:3000/api/drives/dca4331a61ba5f729d1bca586023f65e5e2590b48519cd36aad7be013e0fe9c8'  --header 'Authorization: 92dc55abe6615f3741680ef25863ffae80d2f8924a0c0660b6a46a0c413fe21c'  --header 'Content-Type: application/json' --data-raw '{"deviceId":"sensor1","fields":{"temp":28, "humid":18}}'

