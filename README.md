<p align="center">
  <!-- <a href="" rel="noopener">
 <img width=200px height=200px src="https://i.imgur.com/6wj0hh6.jpg" alt="Project logo"></a> -->
</p>

<h3 align="center">MicroDB</h3>

---

<p align="center"> A lightweight implementation of SQLite3 database system
    <br> 
</p>

## 📝 Table of Contents

- [About](#about)
- [Getting Started](#getting_started)
- [Tests](#tests)
- [Hierarchy](#hierarchy)
- [Built Using](#built_using)
- [TODO](#todo)
- [Authors](#authors)
- [Acknowledgments](#acknowledgement)

## 🧐 About <a name = "about"></a>

This project aims to replicate a full fledged working database management system. We created this to implement our learnings from [CS245](https://www.iitg.ac.in/cse/CS245) and [CS246](https://www.iitg.ac.in/cse/CS246).

The main working file of this project is a CLI (Command Line Interface) similar to MySQL from which we can insert entries as well as select them into and from a single hardcoded table. 

The architechture is based on both an array based implementation wherein the data is stored in pages which are sequential array nodes and also an advanced B+ tree structure where data is stored in pages which represents a single node of the tree structure. The jump from the latter to the newer implementation dramatically enhanced access time and storage optimization.

To add persistence we wrote the whole database on a file on the disk and whenever data is needed it is read into a cache implemented which after doing any operations is flushed onto the dsk again.






[This](/newidea/) folder consists of a newer implementation of this database which supports creating dynamic tables on user input which is yet to be integrated with the main flow of the project.



## 🏁 Getting Started <a name = "getting_started"></a>

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

What things you need to install the software and how to install them.

```
[! fill]
```

### Installing

Open git bash on your local machine and clone the repository

```
git clone https://github.com/AkanshKh/microDB
```

After cloning you're ready to go, run these commands on linux (or wsl)

```
make all
```

```
./tot [database file name]
```



## 🔧 Running the tests <a name = "tests"></a>

Explain how to run the automated tests for this system.

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

<!-- ## 🎈 Usage <a name="usage"></a>

Add notes about how to use the system. -->

<!-- ## 🚀 Deployment <a name = "deployment"></a>

Add additional notes about how to deploy this on a live system. -->

## Directory Hierarchy <a name = "hierarchy"> </a>
```
|—— .gitignore
|—— includes
|    |—— containers.cpp
|    |—— containers.h
|    |—— containers.o
|—— input.txt
|—— inputgen.py
|—— main.cpp
|—— main.o
|—— makefile
|—— mydb.db
|—— newidea
|    |—— a.out
|    |—— checker.cpp
|    |—— checker.exe
|    |—— data.txt
|    |—— heh.txt
|    |—— main.cpp
|    |—— main.h
|    |—— main.o
|    |—— makefile
|    |—— start.cpp
|    |—— start.o
|    |—— tot
|—— private_bustub
|    |—— README.md
|—— pyinput.txt
|—— structures
|    |—— mainpage.cpp
|    |—— maintree.cpp
|    |—— page.cpp
|    |—— page.h
|    |—— tree.cpp
|    |—— tree.h
|—— tot
|—— README.md
```

## ⛏️ Built Using <a name = "built_using"></a>

- [C++](https://www.mongodb.com/) - Database
<!-- - [Express](https://expressjs.com/) - Server Framework
- [VueJs](https://vuejs.org/) - Web Framework
- [NodeJs](https://nodejs.org/en/) - Server Environment -->

## ✍️ Authors <a name = "authors"></a>

- [Akansh Khandelwal](https://github.com/AkanshKh)
- [Anubhab Dutta](https://github.com/anub-dota) 
- [Gautam Sharma]() CSEA Mentor for the project 

## 🎉 Acknowledgements <a name = "acknowledgement"></a>

- [CSEA IITG](https://www.linkedin.com/company/csea-iit-guwahati/?originalSubdomain=in)
- [CS245](), [CS246]()
- [CMU 15-445/645](https://15445.courses.cs.cmu.edu/fall2023/)
- [CMU bustub](https://github.com/cmu-db/bustub)
