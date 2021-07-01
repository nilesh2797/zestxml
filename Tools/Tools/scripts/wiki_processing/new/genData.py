import math

#Assuming input file is tf values only
#IDF is defined as log(|d|/|d_i|) in base 2

#epsilon is for ignoring all those features which have tf-idf very close to zero
eps = 0.01

#Truncation factor: number of points after decimal
trun = 3

#output training files
trnFtMat = 'trnFtMat.0.csv'
trnLblMat = 'trnLblMat.0.csv'

#inputfile for processing
inputFile = 'outfile.txt'

#output indexed file
docIndex = 'docIndex.txt'
ftIndex = 'ftIndex.txt'
lbIndex = 'lbIndex.txt'

ftFile = open(trnFtMat, 'w')
lbFile = open(trnLblMat, 'w')

ftDic = {}
lbDic = {}
ttlDic = {}
idfDic = {}

ftCount = 0
lbCount = 0
docCount = 0

#Function to Dump Dictionaries
def dumpDic(dic, fileName):
	with open(fileName, 'w') as f:
		f.write(str(len(dic)) + '\n')
		for key, val in dic.iteritems():
			f.write(str(key) + '\t' + str(val) + '\n')
	
#Obtain Feature and Label Dictionaries
with open(inputFile, 'r') as f:
	for line in f:
		line = line.strip()
		lineSplit = line.split('\t')
		if(len(lineSplit) < 3):
			continue
		#Obtain Featrues, Labels, Title from line
		title = lineSplit[0]
		lbls = lineSplit[1]
		fts = lineSplit[2]
		#Update Feature dictionary and idf dictionary
		for ft in fts.split(','):
			ft = ft.split(':')
			if(len(ft) < 2):
				continue
			ftr = ft[0].lower()
			if(len(ftr) < 1):
				continue
			if(ftr not in ftDic):
				ftDic[ftr] = ftCount
				ftCount += 1
				idfDic[ftr] = 0
			idfDic[ftr] += 1
		#Update Label
		for lb in lbls.split(','):
			lb = lb.lower()
			if(lb not in lbDic):
				lbDic[lb] = lbCount
				lbCount += 1
		#Add title as label
		title = title.lower()
		ttlDic[title] = docCount
		docCount += 1
		if(title not in lbDic):
			lbDic[title] = lbCount
			lbCount += 1

ftFile.write(str(docCount) + ' ' + str(ftCount) + '\n')
lbFile.write(str(docCount) + ' ' + str(lbCount) + '\n')

#Create Feature and Label Files
with open(inputFile, 'r') as f:
	for line in f:
		line = line.strip()
		lineSplit = line.split('\t')
		if(len(lineSplit) < 3):
			continue
		#Obtain Featrues, Labels, Title from line
		title = lineSplit[0]
		lbls = lineSplit[1]
		fts = lineSplit[2]
		#Create ft file
		for ft in fts.split(','):
			ft = ft.split(':')
			if(len(ft) < 2):
				continue
			ftr = ft[0].lower()
			if(len(ftr) < 1):
				continue
			ftVal = ft[1]
			#convert to tf-idf value
			ftVal = float(ftVal) * math.log(docCount/idfDic[ftr], 2)
			ftVal = round(ftVal, trun)
			if(math.fabs(ftVal - 0) < eps):
				continue
			if(ftr not in ftDic):
				print('PANIC!!!')
			else:
				ftFile.write(str(ftDic[ftr]) + ':' + str(ftVal) + ' ')
		ftFile.write('\n')
		#Create Lb File
		locLb = {}
		for lb in lbls.split(','):
			lb = lb.lower()
			if(lb not in lbDic):
				print('PANIC!!!')
			else:
				lbFile.write(str(lbDic[lb]) + ':' + '1.000' + ' ')
				locLb[lb] = True
		#add title if it wasn't already a label
		title = title.lower()
		if(title not in locLb):
			if(title not in lbDic):
				print('PANIC!!!!!')
			else:
				lbFile.write(str(lbDic[title]) + ':' + '1.000' + ' ')
		lbFile.write('\n')

dumpDic(ttlDic, docIndex)
dumpDic(lbDic, lbIndex)
dumpDic(ftDic, ftIndex)
ftFile.close()
lbFile.close()
