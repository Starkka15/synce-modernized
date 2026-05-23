from pyrapi2 import *
from util import *
from constants import *
import socket

class NoFreeSlotsError(Exception):
    pass

class Partnership:
    def __init__(self, slot, id, guid, hostname, name):
        self.slot = slot
        self.id = id
        self.guid = guid
        self.hostname = hostname
        self.name = name

        self.sync_items = []

    def __str__(self):
        str = ""
        for id in self.sync_items:
            if str:
                str += ", "
            else:
                str = "[ "
            str += SYNC_ITEMS[id][0]
        str += " ]"

        return "P%d: id=%#x, guid=\"%s\", hostname=\"%s\", name=\"%s\", sync_items=%s" % \
            (self.slot, self.id, self.guid, self.hostname, self.name, str)

def get_partnerships(session):
    partnerships = [ None, None ]

    reg_entries = {}
    sync_entries = []

    # Inspect registry entries
    hklm = session.HKEY_LOCAL_MACHINE
    partners_key = hklm.create_sub_key(
            r"Software\Microsoft\Windows CE Services\Partners")
    for pos in xrange(1, 3):
        key = partners_key.create_sub_key("P%d" % pos)
        if key.disposition == REG_OPENED_EXISTING_KEY:
            try:
                id = key.query_value("PId")
                hostname = key.query_value("PName")

                if id != 0 and len(hostname) > 0:
                    reg_entries[hostname] = (pos, id)
            except RAPIError:
                pass
        key.close()
    partners_key.close()

    # Look up the synchronization data on each
    for ctic in config_query_get(session, "Sync", "Sources").children.values():
        sub_ctic = config_query_get(session, "Sync.Sources", ctic.type, recursive=True)

        guid = sub_ctic.type
        hostname = sub_ctic["Server"]
        description = sub_ctic["Name"]

        if hostname in reg_entries:
            pos, id = reg_entries[hostname]
            del reg_entries[hostname]

            pship = Partnership(pos, id, guid, hostname, description)
            partnerships[pos - 1] = pship

            engine = sub_ctic.children["Engines"].children[GUID_WM5_ENGINE]
            for provider in engine.children["Providers"].children.values():
                if int(provider["Enabled"]) != 0:
                    pship.sync_items.append(SYNC_ITEM_ID_FROM_GUID[provider.type])
        else:
            sync_entries.append((guid, hostname, description))

    for entry in reg_entries.values():
        print "deleting dangling registry entry:", entry
        hklm.delete_sub_key(
            r"Software\Microsoft\Windows CE Services\Partners\P%d" % entry[0])

    for entry in sync_entries:
        print "deleting dangling sync source:", entry
        config_query_remove(session, "Sync.Sources", entry[0])

    return partnerships

def create_partnership(session, name, sync_items):
    i = 1
    slot = None
    for partnership in get_partnerships(session):
        if partnership == None:
            slot = i
            break
        i += 1

    if slot == None:
        raise NoFreeSlotsError("all slots are currently full")

    print "adding to slot %d" % slot

    id = generate_id()
    guid = generate_guid()
    hostname = socket.gethostname()

    print "generated id: %#x" % id
    print "generated guid: '%s'" % guid

    #
    # Create the synchronization config data source
    #
    source = Characteristic(guid)
    source["Name"] = name
    source["Server"] = hostname
    source["StoreType"] = "2"

    engines = Characteristic("Engines")
    source.add_child(engines)

    engine = Characteristic(GUID_WM5_ENGINE)
    engines.add_child(engine)

    settings = Characteristic("Settings")
    settings["User"] = "DEFAULT"
    settings["Domain"] = "DEFAULT"
    settings["Password"] = "DEFAULT"
    settings["SavePassword"] = "1"
    settings["UseSSL"] = "0"
    settings["ConflictResolution"] = "1"
    settings["URI"] = "Microsoft-Server-ActiveSync"
    engine.add_child(settings)

    providers = Characteristic("Providers")
    engine.add_child(providers)

    for item_id, item_rec in SYNC_ITEMS.items():
        item_str, item_readonly = item_rec
        item_guid = SYNC_ITEM_ID_TO_GUID[item_id]
        item_enabled = (str(item_id) in sync_items)

        provider = Characteristic(item_guid)
        provider["Enabled"] = str(int(item_enabled))
        provider["ReadOnly"] = str(int(item_readonly))
        provider["Name"] = item_str
        providers.add_child(provider)

    config_query_set(session, "Sync.Sources", source)

    #
    # Update the registry
    #
    hklm = session.HKEY_LOCAL_MACHINE

    partners_key = hklm.create_sub_key(
            r"Software\Microsoft\Windows CE Services\Partners")
    partners_key.set_value("PCur", slot)

    key = partners_key.create_sub_key("P%d" % slot)
    key.set_value("PId", id)
    key.set_value("DataSourceID", guid)
    key.set_value("PName", hostname)
    key.close()

    partners_key.close()

def delete_partnership(session, partnership):
    config_query_remove(session, "Sync.Sources", partnership.guid)

