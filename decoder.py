import csv

def readRecordsFromDB(fileName):
    records = {}
    with open(fileName, 'r') as csvFile:
        dataReader = csv.reader(csvFile)
        for row in dataReader:
            id = int(row.pop(0))
            records[id] = row
    return records

##########################################################################
db = readRecordsFromDB('sample.db')
print(db)