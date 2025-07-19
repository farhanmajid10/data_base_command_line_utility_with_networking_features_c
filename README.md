# Database Command Line Utility in C

This project is a simple command-line database utility written in C for managing employee records. It allows creating a database file, adding employees, searching, removing, updating employee hours, and listing all employees. This was part of a project from Low Level Learning with some added features. I tried to use this to learn how to write Makefiles, use the manpage, setup nvim for c, add nvim plugins, and learn about nvim features that make life easier, along with tmux, shared_libraries, static libraries and so on.  

---

## Features

- Create a new database file with a header
- Add employee records (name, address, hours worked)
- Search employees by name
- Remove employees by name
- Update employee working hours
- List all employees in the database
- Handles data serialization with network byte order conversions
- Uses dynamic memory management for employees list
- Validates file integrity and header consistency

---

## Project Structure

├── bin/ # Compiled executable output

├── include/ # Header files

│ ├── common.h

│ ├── file.h

│ └── parse.h

├── obj/ # Object files from compilation

├── src/ # Source code files

│ ├── file.c

│ ├── main.c

│ └── parse.c

├── my_db.db # Sample database file created during testing

├── Makefile # Build and run automation

└── README.md # This file


---

## Build Instructions

Make sure you have GCC installed.

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-folder>
   ```
Build the project using make:
  ```bash
  make
  ```
Run tests and example commands with:
  ```bash
  make run
  ```

Usage:
Run the compiled executable with the following options:

```bash
./bin/output [OPTIONS]
```
Options
-n
Create a new database file. Requires -f.

-f <file_path>
Specify the database file path. Required for most operations.

-a "<name>,<address>,<hours>"
Add a new employee record.

-l
List all employee records.

-s "<name>"
Search for an employee by name.

-r "<name>"
Remove an employee by name.

-h "<name>,<new_hours>"
Change hours worked for an employee.

Example Workflow
Create a new database:

```bash
./bin/output -f ./my_db.db -n
```

Add employees:
```bash
./bin/output -f ./my_db.db -a "Timmy H.,232 Hartford Ln,124"
./bin/output -f ./my_db.db -a "Simon.,265 Smoky Ln,890"
```

List employees:
```bash
./bin/output -f ./my_db.db -l
```

Search employee:

```bash
./bin/output -f ./my_db.db -s "Timmy H."
```

Update employee hours:

```bash
./bin/output -f ./my_db.db -h "Simon.,100"
```

Remove employee:

```bash
./bin/output -f ./my_db.db -r "Timmy H."
```

## Notes

- The program uses binary file IO and network byte order conversions to store data consistently.
- The database file size and employee count are validated on each run.
- Reallocation of memory is used to dynamically manage the employee list when adding/removing records.
- `Makefile` automates build and testing commands.
