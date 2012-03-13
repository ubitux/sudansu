#!/usr/bin/env python

D = 2
N = D*D
CST_CELL, CST_ROW, CST_COL, CST_BOX, CST_N = range(4+1)
INACTIVE_COLOR = '#333333'

def cell(row, col, color):
    return 'cell_%d_%d [pos="%d,%d!",color="%s"]' % (col, row, col, N*N*N-row, color)

def get_node_column(t, r, c, z):
    if t == CST_CELL:   return c + r*N
    if t == CST_ROW:    return z + r*N
    if t == CST_COL:    return z + c*N
    if t == CST_BOX:    return z + c/D*N + r/D*D*N

def graph(inactive_cells):
    col_counters = [0] * N*N*4
    graph = ''
    for r in range(N):
        for c in range(N):
            for z in range(N):
                row = r*N*N + c*N + z
                color = '%.3f .7 .9' % (float(row) / (N*N*N))

                for cst_type in range(CST_N):
                    col = get_node_column(cst_type, r, c, z)
                    base = cst_type*N*N

                    if inactive_cells:
                        for i in range(col):
                            graph += cell(row, base+i, INACTIVE_COLOR)
                        graph += cell(row, base+col, color)
                        for i in range(N*N - col-1):
                            graph += cell(row, base+col+i+1, INACTIVE_COLOR)
                    else:
                        row = col_counters[base+col]
                        col_counters[base+col] += 1
                        graph += cell(row, base+col, color)
    return graph

if __name__ == '__main__':
    import sys
    print '''graph G {
    bgcolor=black
    node [shape=circle,label="",style=filled,width=.9]
    %s
    }
    ''' % graph(int(sys.argv[1]))
