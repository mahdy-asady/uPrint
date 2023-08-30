import csv
import math
import re
from collections import namedtuple

recordRow = namedtuple('Record', ['formatStr', 'specifiers'])
formatModifiers = namedtuple('formatModifiers', ['flags', 'width', 'precision', 'length', 'specifier'])

def popN(byteList, n):
    data = bytearray()
    for i in range(n):
        data.append(byteList.pop(0))
    return data

##########################################################################
def _readStr(typeSpecifiers, byteList):
    data = ""
    while True:
        char = byteList.pop(0)
        if char == 0:
            break
        data += chr(char)
    return data

def _readDigit(typeSpecifiers, byteList):
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
            # Regex credit belongs to zak @ https://regex101.com/library/rV5bO9?amp%3Bpage=7&orderBy=MOST_POINTS&page=78
            formatSpecifiers = [formatModifiers(*item) for item in re.findall(r'%([-+#0])?(\d+|\*)?(?:\.(\d+|\*))?([hljztL]|hh|ll)?([diuoxXfFeEgGaAcspn])', row[0])]
            records[id] = recordRow(row[0], formatSpecifiers)
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
        specifier = s.specifier
        printArgs += (readerfn[specifier](s, encMsg),)
    print(printArgs)
    print(dbRecord.formatStr % printArgs)

##########################################################################
db = readRecordsFromDB('sample.db')
idLength = getIdByteLength(db)

sampleLog = bytearray(b'\x01\xFF\x00\x00\x00Salam\x00')
decryptMessage(sampleLog, idLength, db)
