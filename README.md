The solver for the challenge is specified via the main.cpp and several includes in the components folder.
The project can be compiled via: "g++ -std=c++20 -O2 main.cpp -o bin/solver"

The final solver takes as input:
- the file of an instance file,
- a config file, and
- destination path for the final solution.

The solver is controlled via the second cell in the main.ipynb, which loops over all inctance folders of a specified path.

