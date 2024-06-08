#include <bits/stdc++.h>
#include <fstream>
using namespace std;

string s, t;
ifstream input_file;
ifstream register_file;
ifstream data_in_file;
ofstream data_out_file;
ofstream output_file;

int registers[16];
int register_dependent[16];
bool done[5];
bool final_done = false;

int memory_address_load = -1;
int memory_address_store = -1;
int memory_decoded_dest = -1;
int total_instructions = 0;
int li_instructions = 0;
int logical_instructions = 0;
int shift_instructions = 0;
int arithmetic_instructions = 0;
int memory_instructions = 0;
int control_instructions = 0;
int halt_instructions = 0;
int total_cycles = -1;
int data_stalls = 0;
int control_stalls = 0;

int pc_val = 0;
int newpc_val = -1;
char opcode = '$';
int evaluated_value = -1;
int temp_register_a = 1000;
int temp_register_b = 1000;
int write_decoded_destination = -1;
int decoded_destination = -1;
bool is_branching = false;
bool execute_branching = false;
bool branching_done = false;

string has_been_decoded = "";
vector<string> instructions;
vector<string> data_cache;

pair<int, int> need_to_write = make_pair(-1, -1);

int hex_to_num(char c)
{ // credit: gfg
    int retval = c;
    if (retval >= 65)
        retval -= 87;
    else
        retval -= 48;
    return retval;
}

string num_to_hex(int n)
{ // credit: gfg
    if (n < 0)
        n += 256;
    int a = n % 16;
    if (a > 9)
        a += 87;
    else
        a += 48;
    int b = n / 16;
    if (b > 9)
        b += 87;
    else
        b += 48;
    string s1 = "";
    s1 = s1 + (char)b;
    s1 = s1 + (char)a;
    return s1;
}

int signed_hex_to_num(char c)
{ // credit: gfg
    int retval = hex_to_num(c);
    if (retval > 7)
        retval -= 16;
    return retval;
}

void write_back()
{
    if (need_to_write.first != -1)
    {
        register_dependent[need_to_write.first]--;
        registers[need_to_write.first] = need_to_write.second;
    }
    if (done[4])
        final_done = true;
    need_to_write = make_pair(-1, -1);
}

void memory_operation()
{
    if (done[3])
        done[4] = true;
    if (memory_address_load != -1)
        registers[memory_decoded_dest] = signed_hex_to_num(data_cache[memory_address_load][0]) * 16 + hex_to_num(data_cache[memory_address_load][1]);
    else if (memory_address_store != -1)
        data_cache[memory_address_store] = num_to_hex(registers[memory_decoded_dest]);
    else if (write_decoded_destination != -1)
        need_to_write = make_pair(write_decoded_destination, evaluated_value);
    else if (execute_branching)
    {
        branching_done = true;
        newpc_val = evaluated_value;
    }
    evaluated_value = -1;
    memory_decoded_dest = -1;
    memory_address_load = -1;
    memory_address_store = -1;
    write_decoded_destination = -1;
}

void execute_stage()
{
    if (done[2])
        done[3] = true;
    if (is_branching)
        execute_branching = true;
    if (temp_register_a == -1)
        return;

    switch (opcode)
    {
    case 'l':
    case 's':
    case '+':
        evaluated_value = temp_register_a + temp_register_b;
        break;
    case '-':
        evaluated_value = temp_register_a - temp_register_b;
        break;
    case '*':
        evaluated_value = temp_register_a * temp_register_b;
        break;
    case '^':
        evaluated_value = temp_register_a ^ temp_register_b;
        break;
    case '&':
        evaluated_value = temp_register_a & temp_register_b;
        break;
    case '|':
        evaluated_value = temp_register_a | temp_register_b;
        break;
    case '~':
        evaluated_value = ~temp_register_a;
        break;
    case '<':
        evaluated_value = temp_register_a << temp_register_b;
        break;
    case '>':
        evaluated_value = temp_register_a >> temp_register_b;
        break;
    case 'i':
        evaluated_value = temp_register_a;
        break;
    case 'j':
        evaluated_value = pc_val + temp_register_a;
        break;
    case 'b':
        if (temp_register_a == 0)
            evaluated_value = pc_val + temp_register_b;
        else
            evaluated_value = pc_val;
        break;

    default:
        evaluated_value = -1;
        break;
    }
    if (opcode != 'l' && opcode != 's' && opcode != 'j' && opcode != 'b')
    {
        write_decoded_destination = decoded_destination;
    }
    else if (opcode == 's')
    {
        memory_address_store = evaluated_value;
        memory_decoded_dest = decoded_destination;
    }
    else if (opcode == 'l')
    {
        memory_address_load = evaluated_value;
        memory_decoded_dest = decoded_destination;
    }

    temp_register_a = 1000;
    temp_register_b = 1000;
    decoded_destination = -1;
}

void instruction_decode()
{
    if (done[1])
        done[2] = true;
    if (has_been_decoded == "")
        return;
    total_instructions++;
    switch (has_been_decoded[0])
    {
    case '0':
        opcode = '+';
        arithmetic_instructions++;
        break;
    case '1':
        opcode = '-';
        arithmetic_instructions++;
        break;
    case '2':
        opcode = '*';
        arithmetic_instructions++;
        break;
    case '3':
        opcode = '+';
        arithmetic_instructions++;
        break;
    case '4':
        opcode = '&';
        logical_instructions++;
        break;
    case '5':
        opcode = '|';
        logical_instructions++;
        break;
    case '6':
        opcode = '^';
        logical_instructions++;
        break;
    case '7':
        opcode = '~';
        logical_instructions++;
        break;
    case '8':
        opcode = '<';
        shift_instructions++;
        break;
    case '9':
        opcode = '>';
        shift_instructions++;
        break;
    case 'a':
        opcode = 'i';
        li_instructions++;
        break;
    case 'b':
        opcode = 'l';
        memory_instructions++;
        break;
    case 'c':
        opcode = 's';
        memory_instructions++;
        break;
    case 'd':
        opcode = 'j';
        control_instructions++;
        break;
    case 'e':
        opcode = 'b';
        control_instructions++;
        break;
    case 'f':
        opcode = 'h';
        halt_instructions++;
        break;

    default:
        break;
    }

    if (opcode != 'j' && opcode != 'b' && opcode != 'h')
        decoded_destination = hex_to_num(has_been_decoded[1]);
    if (opcode == '+' || opcode == '-' || opcode == '*' || opcode == '&' || opcode == '^' || opcode == '|' || opcode == '<' || opcode == '>')
    {
        if (has_been_decoded[0] == '3')
        {
            if (register_dependent[decoded_destination])
            {
                total_cycles++;
                write_back();
                memory_operation();
                data_stalls++;
            }
            if (register_dependent[decoded_destination])
            {
                total_cycles++;
                write_back();
                data_stalls++;
            }
        }
        else
        {
            if (register_dependent[hex_to_num(has_been_decoded[2])] || register_dependent[hex_to_num(has_been_decoded[3])])
            {
                total_cycles++;
                write_back();
                memory_operation();
                data_stalls++;
            }
            if (register_dependent[hex_to_num(has_been_decoded[2])] || register_dependent[hex_to_num(has_been_decoded[3])])
            {
                total_cycles++;
                write_back();
                data_stalls++;
            }
        }

        register_dependent[decoded_destination]++;
        // by now dependancy must be cleared

        if (has_been_decoded[0] == '3')
        {
            temp_register_a = registers[decoded_destination];
            temp_register_b = 1;
        }
        else
        {
            temp_register_a = registers[hex_to_num(has_been_decoded[2])];
            temp_register_b = registers[hex_to_num(has_been_decoded[3])];
        }

        has_been_decoded = "";
    }
    else if (opcode == '~' || opcode == 'l' || opcode == 's')
    {
        if (register_dependent[hex_to_num(has_been_decoded[2])])
        {
            total_cycles++;
            data_stalls++;
            write_back();
            memory_operation();
        }
        if (register_dependent[hex_to_num(has_been_decoded[2])])
        {
            total_cycles++;
            data_stalls++;
            write_back();
        }

        register_dependent[decoded_destination]++;
        if (opcode == '~')
        {
            temp_register_a = registers[hex_to_num(has_been_decoded[2])];
            temp_register_b = -1;
        }
        else
        {
            temp_register_a = registers[hex_to_num(has_been_decoded[2])];
            temp_register_b = signed_hex_to_num(has_been_decoded[3]);
        }

        has_been_decoded = "";
    }
    else if (opcode == 'i')
    {
        temp_register_a = signed_hex_to_num(has_been_decoded[2]) * 16 + hex_to_num(has_been_decoded[3]);
        temp_register_b = -1;
        has_been_decoded = "";
        register_dependent[decoded_destination]++;
    }
    else if (opcode == 'h')
    {
        done[0] = true;
        has_been_decoded = "";
    }
    else
    {
        is_branching = true;
        if (opcode == 'j')
        {
            temp_register_a = signed_hex_to_num(has_been_decoded[1]) * 16 + hex_to_num(has_been_decoded[2]);
            temp_register_b = -1;
        }
        else
        {
            if (register_dependent[hex_to_num(has_been_decoded[1])])
            {
                total_cycles++;
                data_stalls++;
                write_back();
                memory_operation();
            }
            if (register_dependent[hex_to_num(has_been_decoded[1])])
            {
                data_stalls++;
                total_cycles++;
                write_back();
            }
            temp_register_a = registers[hex_to_num(has_been_decoded[1])];
            temp_register_b = signed_hex_to_num(has_been_decoded[2]) * 16 + hex_to_num(has_been_decoded[3]);
        }
        has_been_decoded = "";
    }
}

void instruction_fetch()
{
    if (!is_branching && !done[0])
    {
        pc_val++;
        has_been_decoded = instructions[pc_val];
    }
    else if (done[0])
        done[1] = true;
    else if (branching_done)
    {
        pc_val = newpc_val + 1;
        control_stalls += 2;
        has_been_decoded = instructions[newpc_val];
        is_branching = false;
        execute_branching = false;
        branching_done = false;
        newpc_val = -1;
    }
    else
        return;
}

int main()
{

    for (int i = 0; i < 16; i++)
    {
        registers[i] = 0;
        register_dependent[i] = 0;
        if (i < 5)
            done[i] = 0;
    }

    string append_string;

    input_file.open("ICache.txt");

    while (!input_file.eof())
    {
        getline(input_file, s, '\n');
        getline(input_file, t, '\n');
        append_string = s + t;
        instructions.push_back(append_string);
    }

    input_file.close();

    register_file.open("RF.txt");

    for (int i = 0; i < 16; i++)
    {
        getline(register_file, s, '\n');
        registers[i] = stoi(s);
    }

    register_file.close();

    while (!data_in_file.eof())
    {
        getline(data_in_file, s, '\n');
        data_cache.push_back(s);
    }

    while (!final_done)
    {
        total_cycles++;
        write_back();
        if (final_done)
            break;
        memory_operation();
        execute_stage();
        instruction_decode();
        instruction_fetch();
    }

    float cycle_per_instruction = (float)total_cycles / total_instructions;

    output_file.open("Output.txt");

    output_file << "Total number of instructions execute_staged: " << total_instructions << '\n';
    output_file << "Number of instructions in each class\n";
    output_file << "Arithmetic instructions: " << arithmetic_instructions << '\n';
    output_file << "Logical instructions: " << logical_instructions << '\n';
    output_file << "Shift instructions: " << shift_instructions << '\n';
    output_file << "Memory instructions: " << memory_instructions << '\n';
    output_file << "Load immediate instructions: " << li_instructions << '\n';
    output_file << "Control instructions: " << control_instructions << '\n';
    output_file << "Halt instructions: " << halt_instructions << '\n';
    output_file << "Cycles per instruction: " << cycle_per_instruction << '\n';
    output_file << "Total number of stalls: " << data_stalls + control_stalls << '\n';
    output_file << "Data stalls (RAW): " << data_stalls << '\n';
    output_file << "Control stalls: " << control_stalls << '\n';
    output_file.close();
    data_out_file.open("output/DCache.txt");
    if (!data_out_file) cerr << "Cant open the file" << endl;
    for(int i = 0; i < 256; i++){
        data_out_file << data_cache[i] << '\n';
    }
    data_out_file.close();
    return 0;
}