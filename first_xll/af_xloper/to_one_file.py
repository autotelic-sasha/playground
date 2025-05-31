import regex
import os
import glob
from functools import cmp_to_key

target_file = "../af_xloper_single_header/af_xloper.h"
wc_files = "af_xloper*.h"
file_prefix = """
#pragma once
#define WIN32_LEAN_AND_MEAN   

"""

namespace_prefix = """
namespace autotelica {
namespace xloper {
"""

namespace_suffix = """
}
}
"""

re_content = regex.compile(r"namespace\s+autotelica\s*{\s*namespace\s+xloper\s*{(\s*[\s\S]*)}\s*}")
re_include = regex.compile(r"#include \"(af_xloper[^\"]*)\"")
re_include_other = regex.compile(r"#include .*[\n\r]")

this_folder = os.path.dirname(os.path.realpath(__file__))

class node:
    def __init__(self, filename, parents, content):
        self.filename = filename
        self.parents = parents
        self.content = content

graph = dict()

files = [f for f in glob.glob(wc_files) if f != target_file]
all_includes = []
for fname in files:
    with open(fname) as f:
        all_content = f.read()
    print(f"Processing: {fname}")

    parents = re_include.findall(all_content)
    content = re_content.search(all_content)
    local_includes = [ f.strip() for f in re_include_other.findall(all_content) if "af_xloper" not in f]
    for li in local_includes:
        if li not in all_includes:
            all_includes.append(li)

    if content:
        content = content.groups()[0]
    else:
        content = ""

    graph[fname] = node(fname, parents, content)

stack = []
def tsort(graph, stack, node_key = None):
    if not node_key:
        node_key = list(graph.keys())[0]
    
    if node_key in stack:
        return 

    node = graph[node_key]
    for p in node.parents:
        tsort(graph, stack, p)

    if node_key in stack: 
        raise Exception(f"Circular dependency detected at {node_key}")

    stack.append(node_key)

tsort(graph, stack)

sorted_files = "\n\t".join(stack)
print(f"Sorted files:\n\t{sorted_files}")

all_at_once = file_prefix

all_at_once += "\n".join(all_includes) + "\n"
all_at_once += namespace_prefix

for fname in stack:
    all_at_once += graph[fname].content

all_at_once += namespace_suffix

with open(target_file, "w") as out:
    out.write(all_at_once)

print(f"Joined file writted to {target_file}")