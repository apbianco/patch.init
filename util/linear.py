# S maps to S'
# E maps to E'
#
# f(x) = a.x + b
#
# a.S + b = S'   (1)
# a.E + b = E'   (2)
#
# (1) - (2)
#
# a.(S - E) = (S' - E')
#
#       S'-E'
# a =   ----
#       S-E
#
# a.(S+E) + 2b = S'+E'
#
# 2b = S'+E' - a.(S+E)
#
#            /                    \
#           |        S'-E'         |
# b = 0.5 * |S'+E' - ---- . (S+E)  |
#           |        S-E           |
#            \                    /
#

def linear(a, b, x, coeff=1000.0):
    print("a.x + b = {0}".format(int(coeff * (a*x+b))));

s = -0.01260
e = 0.4664
S = 0
E = 5

a = (S-E)/(s-e)
b = 0.5 * ((S+E) - a * (s+e))
print("a={0}, b={0}".format(a, b));

x = s
while x <= e:
    linear(a, b, x)
    x += 0.01
