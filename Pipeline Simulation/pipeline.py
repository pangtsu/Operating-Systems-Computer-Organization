
import sys 

def format_print(s_register,t_register):    
    # print s_regsiter
    for i in range(0,8,1):
        if(i%4==3):
            print("$s{} = {}".format(i,s_register[i]))
        else:
            print("$s{} = {:<14d}".format(i,s_register[i]),end='')
        
    # print t_register
    for i in range(0,10,1):
        if(i%4==3 or i==9):
            print("$t{} = {}".format(i,t_register[i]))
        else:
            print("$t{} = {:<14d}".format(i,t_register[i]),end='')
                   

def check_hazard(command1,index1,command2):       
    index2 = command2.index(" ")
    index2 += 2   
    return command1[index1:index1+2] == command2[index2:index2+2]

def print_nop(start_stage,end_stage):
    print("nop"+17*" ", end= "")
    count_print = 0
    for i in range(start_stage):
        print("." + 3*" ",end="")
        count_print += 1
    for i in range(end_stage - start_stage + 1):
        if i == 5:
            break
        elif i == 0 and count_print==15:
            print("IF",end="")
        elif i==0:
            print("IF  ",end="")
        elif i == 1 and count_print==15:
            print("ID",end="")        
        elif i == 1:
            print("ID  ",end="")
        elif count_print == 15:
            print("*",end="")
        else:
            print("*   ",end="")
        count_print += 1
    for i in range(15-count_print):
        print(".   ",end="")
    print(".")

def register(s_register,t_register,command):
    # get string representation of v1,v2,fianl_v
    index = command.index(" ")
    index = index + 2
    final_v = command[index:index+2]
    index = index + 4
    v1 = command[index:index+2]
    if v1[0] == "z":
        index = index + 6
    else: 
        index = index + 4
    v2 = command[index:index+2]
    
    # get value of v1,v2
    if v1[0] == "z":
        v1_value = 0
    else:
        n = int(v1[1])
        if v1[0] == "s":
            v1_value = s_register[n]
        else:
            v1_value = t_register[n]
    
    if len(v2)!= 0 and v2[0] == "z":
        v2_value = 0
    else:
        if command[0:4]=="andi" or command[0:4]=="addi" or command[0:3]=="ori" or command[0:4]=="slti":
            index = index - 1
            v2_value = int(command[index:len(command)])
        elif not command[-1].isalpha():
            n = int(v2[1])
            if v2[0] == "s":
                v2_value = s_register[n]
            else:
                v2_value = t_register[n]           
    
    # calcualte value of final_v
    if command[0:3]=="and" or command[0:4]=="andi":
        if v1_value == 0 or v2_value == 0:
            final_value = 0
        else:
            final_value = 1            
    elif command[0:3]=="add" or command[0:4]=="addi":
        final_value = v1_value + v2_value
    elif command[0:2]=="or" or command[0:3]=="ori":
        if v1_value == 0 and v2_value != 0:
            final_value = v2_value
        elif v1_value != 0 and v2_value == 0:
            final_value = v1_value
        elif v1_value == 0 and v2_value == 0:
            final_value = 0
        else:
            final_value = 1
    elif command[0:4]=="slti":
        if v1_value < v2_value:
            final_value = 1
        else:
            final_value = 0
    else:
        # branch command
        return False
    
    # write back into memory
    n = int(final_v[1])
    if final_v[0] == "s":
        s_register[n] = final_value
    else:
        t_register[n] = final_value
    return True 

def check_executed(c,table):
    for i in range(16):
        if table[c][i]!=".":
            return True
    return False


# for submitty
args = sys.argv[1:]
if len(args) != 2:
    exit(-1)

# mode controls N or not N 
mode = sys.argv[1]
# filename is the test file
filename = sys.argv[2]
'''

filename = "ex04.s"
mode = "F"
'''
# open file & read file
fread = open(filename, "r")
commands = []
branch = []
branch_name = []
count_command = 0
for line in fread:
    line = line.strip('\n')
    if line[-1]==":":
        branch_name.append(line[0:len(line)-1])
        branch.append(count_command)
    else:
        commands.append(line.strip('\n'))
        count_command += 1
fread.close()
# initialization
s_register = []
for i in range(8):
    s_register.append(0)
t_register = []
for i in range(10):
    t_register.append(0)
table = []
for i in range(16):
    row = []
    for j in range(16):
        row.append(".")
    table.append(row)


# simulation
print("START OF SIMULATION ", end = "")
if mode == "N":
    print("(no forwarding)")
else:
    print("(forwarding)")

# loop preparation: nop, number of nops of each command
nop = []
for i in range(16):
    nop.append(0)
# loop preparation: status, status of each command
status = []
for i in range(16):
    status.append(0)
# loop preparation: nop_list, help note down the beginning stage of nop
nop_list = []
for i in range(16):
    nop_list.append(0)
# loop preparation: count_finish
count_finish = 0
    
# loop through stages
for stage in range(16):
    print("----------------------------------------------------------------------------------")  
    print("CPU Cycles ===>     1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16" )
    jump = False
    jump_to = ""
    for c in range(len(commands)):        
        if status[c]!= 5 and status[c]!= -10 and c < stage + 1:            
            status[c] += 1
            # there are five status
            if status[c] < 0:
                table[c][stage] = "*"
            elif status[c] == 0:
                status[c] = -10
            elif status[c] == 1:
                table[c][stage] = "IF"
            elif status[c] == 2:
                # check if previous line is nop
                if c >= 1 and table[c-1][stage] == "ID":
                    table[c][stage] = "IF"
                    status[c] -= 1
                else:
                    # normal case
                    table[c][stage] = "ID"
            elif status[c] == 3:
                # most complicated case
                ok1 = True # for variable 1
                ok2 = True # for variable 2
                
                # check variabel 1
                i = commands[c].index(",")
                i = i + 2
                
                double_nop1 = False
                double_nop2 = False
                # need to check c-1, c-2 commands, if exist
                if c>=1:
                    if check_hazard(commands[c],i,commands[c-1]):
                        ok1 = False
                        double_nop1 = True
                        # not it down if not already done so
                        if nop_list[c-1] == 0:
                            nop_list[c-1] = stage - 2
                    if not ok1 and table[c-1][stage]=='.' :
                        ok1 = True
                    if not ok1 and mode == "F":
                        if status[c-1]>=3:
                            ok1 = True

                if c>=2:
                    ok1_before = ok1
                    ok1 = True
                    if check_hazard(commands[c],i,commands[c-2]):
                        ok1 = False
                        if nop_list[c-1] == 0:
                            nop_list[c-1] = stage - 2
                    if not ok1 and table[c-2][stage]=='.' :
                        ok1 = True
                    if not ok1 and mode == "F":
                        if status[c-2]>=3:
                            ok1 = True                     
                    if not ok1 or not ok1_before:
                        ok1 = False
                    else:
                        ok1 = True
                    
                # check variable 2
                i = commands[c].index(",")
                i = i+6
                ok2 = True
                if c>=1:
                    if check_hazard(commands[c],i,commands[c-1]):
                        ok2 = False
                        double_nop2 = True
                        if nop[c-1]==0:
                            nop_list[c-1] = stage - 2
                    if not ok2 and table[c-1][stage]=='.' :
                        ok2 = True
                    if not ok2 and mode == "F":
                        if status[c-1]>=3:
                            ok2 = True

                if c>=2:
                    ok2_before = ok2
                    ok2 = True
                    if check_hazard(commands[c],i,commands[c-2]):
                        ok2 = False
                        if nop_list[c-1] == 0:
                            nop_list[c-1] = stage - 2
                    if not ok2 and table[c-2][stage]=='.' :
                        ok2 = True
                        
                    if not ok2 or not ok2_before:
                        ok2 = False
                    else:
                        ok2 = True 
                    if not ok2 and mode == "F":
                        if status[c-2]>=3:
                            ok2 = True                
                
                if not ok1 or not ok2 :
                    if double_nop1 or double_nop2:
                        nop[c-1] = 2
                    else:
                        nop[c-1] = 1
                    table[c][stage] = "ID"
                    status[c] -= 1
                else:
                    table[c][stage] = "EX"

            if status[c] == 4:
                table[c][stage] = "MEM"
            if status[c] == 5:
                table[c][stage] = "WB"
                count_finish += 1
                # register changes in WB
                if commands[c][-1].isalpha(): 
                    index = commands[c].index(" ")
                    index = index + 2
                    v1 = commands[c][index:index+2]
                    if v1[0] == "z":
                        index = index + 6
                    else: 
                        index = index + 4
                    v2 = commands[c][index:index+2]
                    if v2[0] == "z":
                        index = index + 6
                    else: 
                        index = index + 4
                    jump_to = commands[c][index-1:len(commands[c])]                    
                    
                    if v1[0] == "z":
                        v1_value = 0
                    else:
                        n = int(v1[1])
                        if v1[0] == "s":
                            v1_value = s_register[n]
                        else:
                            v1_value = t_register[n]                                         
                                        
                    if v2[0] == "z":
                        v2_value = 0
                    else:
                        n = int(v2[1])
                        if v2[0] == "s":
                            v2_value = s_register[n]
                        else:
                            v2_value = t_register[n]                    
                        
                    if commands[c][0:3]=="beq":
                        if v1_value == v2_value:
                            jump = True
                    elif commands[c][0:3]=="bne":
                        if v1_value != v2_value:
                            jump = True
                else:
                    register(s_register,t_register,commands[c])

        if not jump:                
            if nop[c-1] >= 1:
                print_nop(nop_list[c-1],stage)
                if nop[c-1] == 2:
                    print_nop(nop_list[c-1],stage)
            if(check_executed(c,table)):
                print(commands[c] + " "*(20-len(commands[c])),end='')
                for i in range(16):
                    if i == 15:
                        print(table[c][i],end="")
                    else:
                        print(table[c][i] + " "*(4-len(table[c][i])),end='')
                print()
        else:
            if(check_executed(c,table)):
                print(commands[c] + " "*(20-len(commands[c])),end='')
                if table[c][stage] != "WB":
                    table[c][stage] = "*"
                    status[c] = status[c] - 6
                for i in range(16):
                    if i == 15:
                        print(table[c][i],end="")
                    else:
                        print(table[c][i] + " "*(4-len(table[c][i])),end='')
                print()                       
            
    if jump:
        c_new = branch[branch_name.index(jump_to)]
        i = c_new
        while i <= c:
            commands.append(commands[i])
            i += 1
        status[c+1] += 1
        table[c+1][stage] = "IF"
        print(commands[c_new] + " "*(20-len(commands[c_new])),end='')
        for i in range(16):
            if i == c+1 and i==15:
                print("IF",end="")
            elif i==c+1:
                print("IF  ",end="")
            elif i == 15:
                print(".",end="")
            else:
                print(".   ",end="")
        print()
        c = c+1
        
    print()
    format_print(s_register,t_register)
    if count_finish == len(commands):
        break
                    
print("----------------------------------------------------------------------------------")
print("END OF SIMULATION")

     