with open("pyinput.txt","w") as f:
    for i in range(1,87):
        f.write(f"insert {i} {i} {i}\n")
    f.write("select\n")
    f.write(".exit\n")
        