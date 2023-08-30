import csv
import math
import re
from collections import namedtuple

recordRow = namedtuple('Record', ['formatStr', 'specifiers'])


def popN(byteList, n):
    data = bytearray()
    for i in range(n):
        data.append(byteList.pop(0))
    return data

##########################################################################
def _readStr(typeIndicator, byteList):
    data = ""
    while True:
        char = byteList.pop(0)
        if char == 0:
            break
        data += chr(char)
    return data

def _readDigit(typeIndicator, byteList):
    num = popN(byteList, 4)
    return int.from_bytes(num, 'little')

readerfn = {
    's':    _readStr,
    'd':    _readDigit
}

##########################################################################

def readRecordsFromDB(fileName):
    records = {}
    with open(fileName, 'r') as csvFile:
        dataReader = csv.reader(csvFile)
        for row in dataReader:
            id = int(row.pop(0))
            records[id] = recordRow(row[0], re.findall(r'(%[^%])', row[0]))
    return records

def getIdByteLength(dict):
    maxValue = max(dict.keys())
    return math.ceil(math.log2(maxValue) / 8)

def decryptMessage(encMsg, idLength, db):
    keyIndex = int.from_bytes(popN(encMsg, idLength), 'little')
    dbRecord = db[keyIndex]
    print(dbRecord.formatStr)
    printArgs = ()
    for s in dbRecord.specifiers:
        printArgs += (readerfn[s[-1]](s[-1], encMsg),)
    print(printArgs)
    print(dbRecord.formatStr % printArgs)

##########################################################################
db = readRecordsFromDB('sample.db')
idLength = getIdByteLength(db)

sampleLog = bytearray(b'\x01\xFF\x00\x00\x00Salam\x00')
decryptMessage(sampleLog, idLength, db)
