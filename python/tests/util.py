import os
import random
from xml.dom import minidom

QUERY_TYPE_GET    = 0
QUERY_TYPE_SET    = 1
QUERY_TYPE_REMOVE = 2

def get_node_on_level(parent, level):
    while level > 0:
        for el in parent.childNodes:
            if el.nodeType == el.ELEMENT_NODE:
                parent = el
                level -= 1
                if level > 0:
                    break
                else:
                    return el

class Characteristic:
    def __init__(self, type, parent=None):
        self.type = type
        self.params = {}
        self.parent = None
        self.children = {}

    def __getitem__(self, key):
        return self.params[key]

    def __setitem__(self, key, value):
        self.params[key] = value

    def __str__(self):
        return self.to_string()

    def to_string(self, level=0):
        indent = "%*s" % (4 * level, "")
        str = "%s* %s\n" % (indent, self.type)
        if self.params:
            for name, value in self.params.items():
                str += "%s    %s = %s\n" % (indent, name, value)
        else:
            str += "%s    [no params]\n" % indent

        for child in self.children.values():
            str += "\n"
            str += child.to_string(level + 1)

        return str

    def add_child(self, ctic):
        self.children[ctic.type] = ctic


def characteristic_tree_from_xml(node):
    return characteristic_from_xml(node)

def characteristic_tree_to_xml(ctic, doc):
    return characteristic_to_xml(ctic, doc)

def characteristic_from_xml(xml_node, parent=None):
    ctic = Characteristic(xml_node.getAttribute("type"), parent)
    for node in xml_node.childNodes:
        if node.nodeType == node.ELEMENT_NODE:
            if node.localName == "characteristic":
                ctic.children[node.getAttribute("type")] = characteristic_from_xml(node, ctic)
            elif node.localName == "parm":
                ctic[node.getAttribute("name")] = node.getAttribute("value")
            else:
                print "unhandled node '%s'" % node.localName

    return ctic

def characteristic_to_xml(ctic, doc, parent=None):
    node = doc.createElement("characteristic")
    node.setAttribute("type", ctic.type)

    for child in ctic.children.values():
        characteristic_to_xml(child, doc, node)

    for name, value in ctic.params.items():
        parm = doc.createElement("parm")
        parm.setAttribute("name", name)
        parm.setAttribute("value", value)
        node.appendChild(parm)

    if parent != None:
        parent.appendChild(node)

    return node

def config_query_get(session, path, leaf, recursive=False):
    return _config_query(session, QUERY_TYPE_GET, path, leaf, recursive)

def config_query_set(session, path, ctic):
    return _config_query(session, QUERY_TYPE_SET, path, None, ctic=ctic)

def config_query_remove(session, path, leaf):
    return _config_query(session, QUERY_TYPE_REMOVE, path, leaf)

def _config_query(session, query_type, path, leaf=None, recursive=False, ctic=None):
    doc = minidom.Document()
    doc_node = doc.createElement("wap-provisioningdoc")
    doc.appendChild(doc_node)

    tokens = path.split(".")

    parent = doc_node
    for token in tokens:
        node = doc.createElement("characteristic")
        node.setAttribute("type", token)
        parent.appendChild(node)
        parent = node

    if query_type == QUERY_TYPE_GET:
        node = doc.createElement("characteristic-query")
        if not recursive:
            node.setAttribute("recursive", "false")
        node.setAttribute("type", leaf)
    elif query_type == QUERY_TYPE_REMOVE:
        node = doc.createElement("nocharacteristic")
        node.setAttribute("type", leaf)
    elif query_type == QUERY_TYPE_SET:
        node = characteristic_tree_to_xml(ctic, doc)

    parent.appendChild(node)

    if os.getenv("SYNCE_DEBUG"):
        print "CeProcessConfig request:"
        print doc_node.toprettyxml()

    reply = session.process_config(doc_node.toxml(), 1)
    reply_doc = minidom.parseString(reply)

    if os.getenv("SYNCE_DEBUG"):
        print "CeProcessConfig response:"
        print reply_doc.documentElement.toprettyxml()

    reply_node = get_node_on_level(reply_doc, 2 + len(tokens))

    if query_type != QUERY_TYPE_SET:
        return characteristic_tree_from_xml(reply_node)
    else:
        return None

def generate_id():
    w1 = random.randint(0x0, 0xFFFF)
    w2 = random.randint(0x0, 0xFFFF)
    return (w1 << 16) | w2

def generate_guid():
    d1 = random.randint(0, 0xFFFFFFFF)
    d2 = random.randint(0, 0xFFFF)
    d3 = random.randint(0, 0xFFFF)
    d4 = []
    for i in range(8):
        d4.append(random.randint(0, 0xFF))

    guid = "{%08X-%04X-%04X-" % (d1, d2, d3)
    for i in xrange(len(d4)):
        guid += "%02X" % d4[i]
        if i == 1:
            guid += "-"
    guid += "}"

    return guid

