#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "wchar.h"
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH_OF_COMMAND 128
#define LENGTH_OF_BIRTH_DATE 11
#define SURNAME_LEN 128
#define NAME_LEN 32
#define HOSPITAL_ADDRESS_LEN 64 
#define PHONE_LEN 52
#define MAX_LEN_OF_FIELD 15

typedef struct Doctor {
	int id;
	char name[NAME_LEN];
	char surname[SURNAME_LEN];
	char dateOfBirth[LENGTH_OF_BIRTH_DATE];
	int experience;
	int nextDoctorID;
	int isDelete;
} doctor;

typedef struct Hospital {
	int id;
	int budget;	
	char address[HOSPITAL_ADDRESS_LEN];
	char phone[PHONE_LEN];
 	int firstDoctorID;
} hospital;

typedef struct FileHandler {
	FILE* dataIndexFile;
	FILE* hospitalDataFile;
	FILE* doctorDataFile;
	char* indexFileName;
	char* dataFileName;
	char* doctorFileName;

}fileHandler;

fileHandler handler;

void updateMasterFileRecord(char [MAX_LENGTH_OF_COMMAND], char*);
int searchForMasterIndex(int);
hospital hospitalWithIndex(int);
void changeHospitalRecord(hospital*, char*);
void updateRecord(int, hospital*);
void deleteEndOfLine(char*);
void printMenu();
void getRecordFromSlaveFile(char[MAX_LENGTH_OF_COMMAND], char*);
void insertRecordToSlaveFile(char[MAX_LENGTH_OF_COMMAND], char*);
void writeToSlaveFile(doctor*);
void appendRecordToMaster(int);
doctor searchInSlaveFile(int);
void appendAfterLastDoctorInHospital(doctor*, doctor*);
void printSlaveRecord(doctor*);
void rewriteHospitalRecord(hospital, int);
void rewriteDoctorRecord(doctor*);
void updateSlaveRecord(char[MAX_LENGTH_OF_COMMAND], char*);
void changeSlaveRecord(doctor*);
void deleteMasterRecord(char[MAX_LENGTH_OF_COMMAND], char*);
void deleteRecordFromIndexFile(int);
void deleteAllSubrecords(int);
void clearContentOfFiles();


int doesFileHasContent(FILE* f) {
	fseek(f, 0, SEEK_END);
	unsigned long length = ftell(f);
	if (length != 0) return 1;
	return 0;
}


void showMasterFile() {
	hospital hospital;
	int i = 1;
	handler.hospitalDataFile = fopen(handler.dataFileName, "rb");
	fseek(handler.hospitalDataFile, 0L, SEEK_SET);
	for (;;){
		fread(&hospital, sizeof(hospital), 1, handler.hospitalDataFile);
		if (feof(handler.hospitalDataFile)) break;
		printf("%d. %d %d %s %s\n", i, hospital.id, hospital.budget, hospital.address, hospital.phone);
		++i;
	}
	fclose(handler.hospitalDataFile);
}

void insertToIndexFile(hospital *newHospital) {
	handler.dataIndexFile = fopen(handler.indexFileName, "rb+");
	int oldIndex;
	fread(&oldIndex, sizeof(int), 1, handler.dataIndexFile);
	//newHospital->id = oldIndex * oldIndex;
	fseek(handler.dataIndexFile, 0L, SEEK_SET);
	int newIndex = oldIndex + 1;
	fwrite(&newIndex, sizeof(int), 1, handler.dataIndexFile);
	fseek(handler.dataIndexFile, 0L, SEEK_END);
	fwrite(&(newHospital->id), sizeof(int), 1, handler.dataIndexFile);
	fwrite(&oldIndex, sizeof(int), 1, handler.dataIndexFile);
	fclose(handler.dataIndexFile);
}


void insertToMasterFile(char command[MAX_LENGTH_OF_COMMAND]) {
	char delims[] = " \n";
	hospital newHospital;
	char *delim = strtok(command, delims);
	int id = atoi(strtok(NULL, delims));
	newHospital.id = id;
	char* budgetString = strtok(NULL, delims);
	newHospital.budget = atoi(budgetString);
	strncpy(newHospital.address, strtok(NULL, delims), HOSPITAL_ADDRESS_LEN);
	strncpy(newHospital.phone, strtok(NULL, delims), PHONE_LEN);
	newHospital.firstDoctorID = -1;
	insertToIndexFile(&newHospital);

	handler.hospitalDataFile = fopen(handler.dataFileName, "ab+");
	fwrite(&newHospital, sizeof(hospital), 1, handler.hospitalDataFile);
	fclose(handler.hospitalDataFile);
}


void printMasterRecord(int index) {
	hospital hospital;
	handler.hospitalDataFile = fopen(handler.dataFileName, "rb+");
	fseek(handler.hospitalDataFile, index * sizeof(hospital), SEEK_SET);
	fread(&hospital, sizeof(hospital), 1, handler.hospitalDataFile);
	fclose(handler.hospitalDataFile);
	printf("ID: %d Budget: %d Address: %s  Phone: %s\n", hospital.id, hospital.budget, hospital.address, hospital.phone);
}


void getRecordFromMasterFile(char command[MAX_LENGTH_OF_COMMAND]) {
	char* delims = " \n";
	char* option = strtok(command, delims);
	int masterID = atoi(strtok(NULL, delims));
	int masterIndex = searchForMasterIndex(masterID);
	
	printMasterRecord(masterIndex);
}


void handleCommand() {
	char *delims = " \n";
	char command[MAX_LENGTH_OF_COMMAND];
	fgets(command, MAX_LENGTH_OF_COMMAND, stdin);
	char copyOfcommand[MAX_LENGTH_OF_COMMAND];
	memcpy(copyOfcommand, command, MAX_LENGTH_OF_COMMAND);
	char* option = strtok(command, delims);
 	if (!strcmp(option, "show-m")) {
		showMasterFile();
	}
	else if (!strcmp(option, "insert-m")) {
		insertToMasterFile(&copyOfcommand);
	}
	else if (!strcmp(option, "get-m")) {
		getRecordFromMasterFile(&copyOfcommand);
	}
	else if (!strcmp(option, "update-m")) {
		updateMasterFileRecord(&copyOfcommand, delims);
	}
	else if (!strcmp(option, "insert-s")) {
		insertRecordToSlaveFile(copyOfcommand, delims);
	}
	else if (!strcmp(option, "get-s")) {
		getRecordFromSlaveFile(copyOfcommand, delims);
	}
	else if (!strcmp(option, "update-s")) {
		updateSlaveRecord(copyOfcommand, delims);
	}
	else if (!strcmp(option, "delete-m")) {
		deleteMasterRecord(copyOfcommand, delims);
	}else if (!strcmp(option, "clear")) {
		clearContentOfFiles();
	}
}


int main(int argc, char** argv) {
	printMenu();
	handler.indexFileName = "hospital.ind";
	handler.dataFileName = "hospital.dat";
	handler.doctorFileName = "doctor.dat";
	handler.dataIndexFile = fopen(handler.indexFileName, "ab");
	handler.doctorDataFile = fopen(handler.doctorFileName, "ab+");
	fclose(handler.doctorDataFile);
	if (!doesFileHasContent(handler.dataIndexFile)) {
		int firstIndex = 0;
		fwrite(&firstIndex, sizeof(int), 1, handler.dataIndexFile);
	}
	fclose(handler.dataIndexFile);
	while (1) {
		handleCommand();
	}
	return 0;	
}

void printMenu() {

	printf("This is a hospital database management programm\n");
	printf("You have next commands available for input:\n");
	printf("1.\"show-m\" - to look information about all hospital in database\n");
	printf("2.\"get-m ID\" - to look information about hospital with ID key\n");
	printf("3.\"show-s ID\" - to look information about all doctors in hospital with ID key\n");
	printf("4.\"get-s hospitalID docID\" - to look information about doctors with docID key in hospital with hospitalID key\n");
	printf("5.\"insert-m ID budget address phone\" - to add new hospital\n");
	printf("6.\"update-m ID\" - to update record of hospital with ID\n");

}

int searchForMasterIndex(int masterID) {

	handler.dataIndexFile = fopen(handler.indexFileName, "rb");
	fseek(handler.dataIndexFile, sizeof(int), SEEK_SET);
	int masterIndex = -1;
	for (;;) {
		int number;
		fread(&number, sizeof(int), 1, handler.dataIndexFile);
		if (feof(handler.dataIndexFile)) break;
		if (number == masterID) {
			fread(&masterIndex, sizeof(int), 1, handler.dataIndexFile);
			break;
		}
		fseek(handler.dataIndexFile, sizeof(int), SEEK_CUR);
	}
	fclose(handler.dataIndexFile);
	return masterIndex;
}

void updateMasterFileRecord(char command[MAX_LENGTH_OF_COMMAND], char* delims) {
	
	char field[MAX_LEN_OF_FIELD];
	char* delim = strtok(command, delims);
	int id = atoi(strtok(NULL, delims));
	int masterIndex = searchForMasterIndex(id);
	
	hospital hospital = hospitalWithIndex(masterIndex);
	printf("Which field of record would you like to update?\n");
	fgets(field, MAX_LEN_OF_FIELD, stdin);
	deleteEndOfLine(field);
	changeHospitalRecord(&hospital, field);
	updateRecord(masterIndex, &hospital);
}

hospital hospitalWithIndex(int index) {
	handler.hospitalDataFile = fopen(handler.dataFileName, "rb+");
	hospital hospital;
	fseek(handler.hospitalDataFile, index * sizeof(hospital), SEEK_SET);
	fread(&hospital, sizeof(hospital), 1, handler.hospitalDataFile);
	fclose(handler.hospitalDataFile);
	return hospital;
}

void changeHospitalRecord(hospital *hospital, char* field) {
	if (!strcmp(field, "Address")) {
		fgets(hospital->address, HOSPITAL_ADDRESS_LEN, stdin);
		deleteEndOfLine(hospital->address);
	}
	else if (!strcmp(field, "Phone")) {
		fgets(hospital->phone, PHONE_LEN, stdin);
		deleteEndOfLine(hospital->phone);
	}
	else if (!strcmp(field, "Budget")) {
		scanf("%d", &hospital->budget);
		char c = getchar();
	}
	else {
		printf("You can update only existing fields");
	}
}

void updateRecord(int index, hospital* hospital) {
	handler.hospitalDataFile = fopen(handler.dataFileName, "rb+");
	fseek(handler.hospitalDataFile, index * sizeof(*hospital), SEEK_SET);
	fwrite(hospital, sizeof(*hospital), 1, handler.hospitalDataFile);
	fclose(handler.hospitalDataFile);
}

void deleteEndOfLine(char* string) {
	char* ptr = string;
	if ((ptr = strchr(string, '\n')) != NULL)
		*ptr = '\0';

}

void getRecordFromSlaveFile(char command[MAX_LENGTH_OF_COMMAND], char* delims) {
	char field[MAX_LEN_OF_FIELD];
	char* delim = strtok(command, delims);
	int id = atoi(strtok(NULL, delims));
	char* docIDString = strtok(NULL, delims);
	doctor doc;
	int index = searchForMasterIndex(id);
	hospital hospital = hospitalWithIndex(index);

	if (docIDString != NULL) {
		int docID = atoi(docIDString);
		doc = searchInSlaveFile(hospital.firstDoctorID);
		while (doc.id != docID && doc.nextDoctorID != -1) {
			doc = searchInSlaveFile(doc.nextDoctorID);
			
		}
		printSlaveRecord(&doc);
	}
	else {
		if (hospital.firstDoctorID != -1) {
			doc = searchInSlaveFile(hospital.firstDoctorID);
			while(doc.id != -1){
				printSlaveRecord(&doc);
				doc = searchInSlaveFile(doc.nextDoctorID);
			}
		}
	}
}
void insertRecordToSlaveFile(char command[MAX_LENGTH_OF_COMMAND], char* delims) {
	
	char* delim = strtok(command, delims);
	doctor doctor;
	int masterID = atoi(strtok(NULL, delims));
	int docID = atoi(strtok(NULL, delims));
	doctor.id = docID;
	strncpy(doctor.name, strtok(NULL, delims), NAME_LEN);
	strncpy(doctor.surname, strtok(NULL, delims), SURNAME_LEN);
	strncpy(doctor.dateOfBirth, strtok(NULL, delims), LENGTH_OF_BIRTH_DATE);
	int experience = atoi(strtok(NULL, delims));
	doctor.experience = experience;
	doctor.nextDoctorID = -1;
	appendRecordToMaster(masterID, &doctor);
	writeToSlaveFile(&doctor);
}


void appendRecordToMaster(int masterID, doctor *doc) {
	int index = searchForMasterIndex(masterID);
	hospital hospital = hospitalWithIndex(index);
	if (hospital.firstDoctorID != -1) {
		doctor firstDoctor = searchInSlaveFile(hospital.firstDoctorID);
		appendAfterLastDoctorInHospital(&firstDoctor, doc);
	}
	else {
		hospital.firstDoctorID = doc->id;
		rewriteHospitalRecord(hospital, index);
	}
}

void rewriteHospitalRecord(hospital hosp, int index) {
	handler.hospitalDataFile = fopen(handler.dataFileName, "rb+");
	fseek(handler.hospitalDataFile, index * sizeof(hospital), SEEK_SET);
	fwrite(&hosp, sizeof(hospital), 1, handler.hospitalDataFile);
	fclose(handler.hospitalDataFile);
}

void writeToSlaveFile(doctor* doc) {
	
	handler.doctorDataFile = fopen(handler.doctorFileName, "ab+");
	fwrite(doc, sizeof(doctor), 1, handler.doctorDataFile);
	fclose(handler.doctorDataFile);
}

doctor searchInSlaveFile(int id) {
	doctor doc;
	handler.doctorDataFile = fopen(handler.doctorFileName, "rb");
	for (;;) {
		fread(&doc, sizeof(doctor), 1, handler.doctorDataFile);
		if (feof(handler.doctorDataFile)) break;
		if (doc.id == id) {
			fclose(handler.doctorDataFile);
			return doc;
		}
	}
	doc.id = -1;
	fclose(handler.doctorDataFile);
	return doc;
}

void appendAfterLastDoctorInHospital(doctor* firstDoc, doctor* newDoc) {
	while (firstDoc->nextDoctorID != -1) {
		doctor doc = (searchInSlaveFile(firstDoc->nextDoctorID));
		firstDoc = &doc;
	}
	firstDoc->nextDoctorID = newDoc->id;
	handler.doctorDataFile = fopen(handler.doctorFileName, "rb+");
	for (;;) {
		doctor oldRecord;
		fread(&oldRecord, sizeof(doctor), 1, handler.doctorDataFile);
		if (feof(handler.doctorDataFile)) break;
		if (oldRecord.id == firstDoc->id) {
			oldRecord.nextDoctorID = firstDoc->nextDoctorID;
			fseek(handler.doctorDataFile, -1L * (int)sizeof(doctor), SEEK_CUR);
			fwrite(&oldRecord, sizeof(doctor), 1, handler.doctorDataFile);
			fclose(handler.doctorDataFile);
			break;
		}
	}

}


void printSlaveRecord(doctor* doc) {
	printf("ID: %d Name: %s Surname: %s Date Of Birth: %s Experience: %d\n", doc->id, doc->name, doc->surname, doc->dateOfBirth, doc->experience);
}

void updateSlaveRecord(char command[MAX_LENGTH_OF_COMMAND], char* delims) {
	char* option = strtok(command, delims);
	int masterID = atoi(strtok(NULL, delims));
	int doctorID = atoi(strtok(NULL, delims));
	int hospitalIndex = searchForMasterIndex(masterID);
	hospital hosp = hospitalWithIndex(hospitalIndex);
	doctor doc = searchInSlaveFile(hosp.firstDoctorID);
	while (doc.id != doctorID && doc.id != -1) {
		doc = searchInSlaveFile(doc.nextDoctorID);
	}
	char field[MAX_LEN_OF_FIELD];
	fgets(field, MAX_LEN_OF_FIELD, stdin);
	deleteEndOfLine(field);
	changeSlaveRecord(&doc, field);
	handler.doctorDataFile = fopen(handler.doctorFileName, "rb+");
	for (;;) {
		doctor oldRecord;
		fread(&oldRecord, sizeof(doctor), 1, handler.doctorDataFile);
		if (feof(handler.doctorDataFile)) break;
		if (oldRecord.id == doc.id) {
			fseek(handler.doctorDataFile, -1L * (int)sizeof(doctor), SEEK_CUR);
			fwrite(&doc, sizeof(doctor), 1, handler.doctorDataFile);
			fclose(handler.doctorDataFile);
			break;
		}
	}

}

void changeSlaveRecord(doctor* doc, char* field) {
	if (!strcmp(field, "Name")) {
		fgets(doc->name, NAME_LEN, stdin);
		deleteEndOfLine(doc->name);
	}
	else if (!strcmp(field, "Surname")) {
		fgets(doc->surname, SURNAME_LEN, stdin);
		deleteEndOfLine(doc->surname);
	}
	else if (!strcmp(field, "Experience")) {
		scanf("%d", &doc->experience);
		char c = getchar();
	}
	else if (!strcmp(field, "Date Of Birth")) {
		fgets(doc->dateOfBirth, LENGTH_OF_BIRTH_DATE, stdin);
		deleteEndOfLine(doc->dateOfBirth);
	}
	else {
		printf("You can update only existing fields");
	}
}

void deleteMasterRecord(char command[MAX_LENGTH_OF_COMMAND], char* delims) {
	char* option = strtok(command, delims);
	int masterID = atoi(strtok(NULL, delims));
	handler.hospitalDataFile = fopen(handler.dataFileName, "rb");
	FILE* hospitalDataTmp = fopen("hospital_tmp.dat", "wb");
	hospital toDelete;
	int found = 0;
	int firstDocId = -1;
	for (;;) {
		fread(&toDelete, sizeof(hospital), 1, handler.hospitalDataFile);
		if (feof(handler.hospitalDataFile)) break;
		if (toDelete.id == masterID) {
			printf("Record found and deleted\n\n");
			firstDocId = toDelete.firstDoctorID;
			found = 1;
		}
		else {
			fwrite(&toDelete, sizeof(hospital), 1, hospitalDataTmp);
		}
	}
	if (!found) printf("No records with requested id: %d\n", masterID);
	fclose(handler.hospitalDataFile);
	fclose(hospitalDataTmp);
	remove(handler.dataFileName);
	rename("hospital_tmp.dat", handler.dataFileName);
	deleteRecordFromIndexFile(masterID);
	deleteAllSubrecords(firstDocId);
}

void deleteRecordFromIndexFile(int id) {
	handler.dataIndexFile = fopen(handler.indexFileName, "rb");
	FILE* hospitalIndexFileTmp = fopen("hospital_tmp.ind", "wb");
	int index;
	int masterID;
	int found = 0;
	fseek(handler.dataIndexFile, sizeof(int), SEEK_SET);
	for (;;) {
		fread(&masterID, sizeof(int), 1, handler.dataIndexFile);
		fread(&index, sizeof(int), 1, handler.dataIndexFile);
		if (feof(handler.dataIndexFile)) break;
		if  (masterID == id) {
			printf("Record in index found and deleted\n\n");
			found = 1;
		}
		else {
			fwrite(&masterID, sizeof(int), 1, hospitalIndexFileTmp);
			fwrite(&index, sizeof(int), 1, hospitalIndexFileTmp);
		}
	}
	if (!found) printf("No records in index file with requested id: %d\n", id);
	fclose(handler.dataIndexFile);
	fclose(hospitalIndexFileTmp);
	remove(handler.indexFileName);
	rename("hospital_tmp.ind", handler.indexFileName);
}

void deleteAllSubrecords(int firstDocID) {
	if (firstDocID != -1) {
		handler.doctorDataFile = fopen(handler.doctorFileName, "rb");
		doctor doc;
		FILE* doctorsDataTmp = fopen("doctor_tmp.dat", "wb");
		for (;;) {
			fread(&doc, sizeof(doctor), 1, handler.doctorDataFile);
			if (feof(handler.doctorDataFile)) break;
			if (doc.id == firstDocID) {
				printf("Delete subrecord\n");
				firstDocID = doc.nextDoctorID;
			}
			else {
				fwrite(&doc, sizeof(doctor), 1, doctorsDataTmp);
			}
		}
		fclose(handler.doctorDataFile);
		fclose(doctorsDataTmp);
	}
	remove(handler.doctorFileName);
	rename("doctor_tmp.dat", handler.doctorFileName);

}

void clearContentOfFiles() {
	handler.hospitalDataFile = fopen(handler.dataFileName, "wb");
	fclose(handler.hospitalDataFile);
	handler.dataIndexFile = fopen(handler.indexFileName, "wb");
	fclose(handler.dataIndexFile);
	handler.doctorDataFile = fopen(handler.doctorFileName, "wb");
	fclose(handler.doctorDataFile);
}