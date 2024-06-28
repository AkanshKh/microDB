import subprocess
import os
def delete_file(file_path):
    try:
        os.remove(file_path)
        # print(f"File {file_path} deleted successfully.")
        return True
    except FileNotFoundError:
        # print(f"File {file_path} not found.")
        return False
    except PermissionError:
        # print(f"Permission denied: cannot delete {file_path}.")
        return False
    except Exception as e:
        # print(f"Error deleting file {file_path}: {e}")
        return False

def test_normal_flow()->bool:
    delete_file("test.db")
    process = subprocess.Popen(["./tot","test.db"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    for i in range(8):
        process.stdin.write(f"insert {i} John johndoe@doemail.com\n")
        process.stdin.flush()
    process.stdin.write("select\n")
    process.stdin.flush()
    process.stdin.write(".exit\n")

    stdout, stderr = process.communicate()
    
    # Print the results for debugging purposes
    # print("Results:\n", stdout)
    # print("Errors:\n", stderr)
    if(stderr!="" or ("db > error:" in stdout)):
        return False
    else:
        return True

def test_duplicate_key()->bool:
    delete_file("test.db")
    process = subprocess.Popen(["./tot","test.db"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    process.stdin.write("insert 1 John jjonahson@gmail.com\n")
    process.stdin.write("insert 2 John jjonahson@gmail.com\n")
    process.stdin.write("insert 1 John jjonahson@gmail.com\n")
    process.stdin.flush()
    process.stdin.write(".exit\n")

    stdout,stderr = process.communicate()
    # print(stdout)
    if stderr!="" or "db > error: Duplicate key" in stdout:
        return True
    return False

def test_params()->bool:
    # check if not passing a filenames triggers any unwanted behaviour
    process = subprocess.Popen(["./tot"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if(process.stdout.read().strip()=="Must supply a database filename."):
        return True
    return False

def test_normal_flow_big()->bool:
    delete_file("test.db")
    process = subprocess.Popen(["./tot","test.db"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    for i in range(80):
        process.stdin.write(f"insert {i} John johndoe@doemail.com\n")
        process.stdin.flush()
    process.stdin.write("select\n")
    process.stdin.flush()
    process.stdin.write(".exit\n")

    stdout, stderr = process.communicate()
    
    # Print the results for debugging purposes
    # print("Results:\n", stdout)
    # print("Errors:\n", stderr)
    if(stderr!="" or ("db > error:" in stdout)):
        return False
    else:
        return True


def tests_master():
    # run all tests from here
    #print success or failures for the tests
    all_tests_passed=True

    if test_params()==False:
        print("Failed Test: Input parameters for execution")
        all_tests_passed=False
    if test_duplicate_key()==False:
        print("Failed Test: Duplicate key check")
        all_tests_passed=False
    if test_normal_flow()==False:
        print("Failed Test: Normal Execution Flow")
        all_tests_passed=False
    if test_normal_flow_big()==False:
        print("Failed Test: Invalid input to program")
        all_tests_passed=False
    
    if all_tests_passed:
        print("All tests passed successfully")
    delete_file("test.db")

if __name__=="__main__":
    tests_master()