from sys import stdin, argv


def push_ref():
    global ip
    adr = mem[ip]
    stack.append(adr)
    if adr >= boundary:
        mem[adr + 4] += 1
    ip += 1

def gc():
    global Y
    to_collect = [stack.pop()]
    while to_collect:
        n = to_collect.pop()
        if not isinstance(n, int) or n < boundary:
            continue
        mem[n + 4] -= 1
        if mem[n + 4] > 0:
            continue
        for i in range(3):
            to_collect.append(mem[n+i])
        mem[n] = Y
        Y = n

def cell(a, b, c):
    global Y, mem
    if Y:
        t = Y
        Y = mem[Y]
    else:
        # print('allocating at', len(mem))
        t = len(mem)
        mem += [None, None, None, ret, None]
    mem[t] = a
    mem[t+1] = b
    mem[t+2] = c
    mem[t+4] = 1
    return t


def cons():
    c = stack.pop()
    stack.append(cell(push_ref, stack.pop(), c))


def take():
    stack.append(cell(stack.pop(), push_ref, stack.pop()))


def dup():
    if stack[-1] >= boundary:
        mem[stack[-1] + 4] += 1
    stack.append(stack[-1])


def k():
    global ip
    n = stack.pop()
    gc()
    rs.append(ip)
    rs.append(n)
    rs.append(0)
    ip = n

def post_k():
    stack.append(rs.pop())
    gc()
    ret()


def number():
    global ip
    rs.append(mem[ip])
    rs.append(stack.pop())
    ip = 1


def post_number():
    global ip
    adr, n = rs.pop(), rs.pop()
    if not n:
        stack.append(adr)
        gc()
        ret()
        return
    n -= 1
    rs.append(n)
    rs.append(adr)
    rs.append(1)
    ip = adr


def inp():
    stack.append(cell(number, ord(stdin.read(1)), 0))


def out():
    global ip, out_val
    out_val = 0
    adr = stack.pop()
    rs.append(adr)
    rs.append(2)
    stack.append(3)
    ip = adr


def post_out():
    print(chr(out_val), end='')
    stack.append(rs.pop())
    gc()
    ret()


def counter():
    global out_val
    out_val += 1
    ret()


def quote():
    global ip
    stack.append(ip)
    n = 1
    while n:
        val = mem[ip]
        ip += 1
        if val == quote:
            n += 1
        elif val == ret:
            n -= 1


def ret():
    global ip
    ip = rs.pop()


def mainloop():
    global ip, step
    while rs:
        step += 1
        com = mem[ip]
        rep = com.__name__ if hasattr(com, '__name__') else str(com)
        # print(rep, 'at', ip, 'stack:', stack, 'rs:', rs)
        ip += 1
        if isinstance(com, int):
            rs.append(ip)
            ip = com
        else:
            com()
        # if step % 10000 == 0:
        #     verify()


g = {
    '+': dup,
    '-': k,
    '>': cons,
    '<': take,
    '.': out,
    ',': inp,
    '[': quote,
    ']': ret,
}


def verify():
    print('Check at', step)
    v = {i: None for i in range(boundary, len(mem), 5)}
    # print(v)
    n = Y
    while n:
        v[n] = 0
        n = mem[n]
    to_check = stack + [x - (x - boundary) % 5 for x in rs[1:] + [ip]]
    while to_check:
        n = to_check.pop()
        if n >= boundary:
            if not v[n]:
                if v[n] == 0:
                    print('Collected a cell in use:', n)
                v[n] = 1
                for i in range(3):
                    if isinstance(mem[n + i], int) and mem[n + i] >= boundary:
                        # print('adding check for', mem[n + i])
                        to_check.append(mem[n + i])
            else:
                v[n] += 1
    for k, v in v.items():
        if v is None:
            print('Discrepancy in cell', k, ':', v, 'vs', mem[k + 4])


if __name__ == '__main__':
    mem = [post_k, post_number, post_out, counter]
    with open(argv[1]) as f:
        while c := f.read(1):
            if c in g: mem.append(g[c])
    step = 0
    mem.append(ret)
    stack = []
    rs = [100000000000]
    ip = 4
    Y = None
    boundary = len(mem)
    mainloop()
    ip = 0
    verify()
    print('boundary:', boundary, 'allocated:', (len(mem) - boundary) / 5,
          'stack:', len(stack))
