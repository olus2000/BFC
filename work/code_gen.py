# Generate BLC program for writing `s` given that `print 0` is n+2 definitions
# above and `print 1` is n+1 definitions above.
def make(s, n):
    ans = []
    for i in s:
        if i == '0':
            ans.append('01 11' + '1' * n + '0')
        elif i == '1':
            ans.append('01 1' + '1' * n + '0')
    return ' '.join(ans)


# Generate the quine printing code given that `call top` is 1 definition above,
# and `call bottom` is 2 definitions above
def stend(s):
    ans = []
    for i in s:
        if i == '0':
            ans.append('0110')
        elif i == '1':
            ans.append('01110')
    return ''.join(ans)


# Given a list of target numbers and a BLC string to generate them from return
# the representations of these numbers with an additional zero at the start and
# the unconsumed part of the BLC string
def numerize(target, source):
    sp = 0
    answer = ['']
    for i in target:
        i = str(i)
        j = ''.join(str(int(n) % 2) for n in i)
        n = source.find(j, sp)
        if n == -1: raise Exception()
        if n > sp: answer[-1] += '.' + source[sp:n]
        answer.append(i)
        sp = n + len(i)
    return answer, source[sp:]
