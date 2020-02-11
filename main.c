#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "wchar.h"
#include <locale.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH_OF_COMMAND 128
#define LENGTH_OF_BIRTH_DATE 8
#define SURNAME_LEN 128
#define NAME_LEN 32
#define HOSPITAL_ADDRESS_LEN 64 
#define PHONE_LEN 52
#define MAX_LEN_OF_FIELD 15

typedef struct Doctor {
	int id;
	char dateOfBirth[LENGTH_OF_BIRTH_DATE];
	char name[NAME_LEN];
	char surname[SURNAME_LEN];
	int experience;
	int nextDoctorID;
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

	handler.hospitalDataFile = fopen(handler.dataFileName, "a");
	fseek(handler.hospitalDataFile, 0L, SEEK_END);
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
}


int main(int argc, char** argv) {
	printMenu();
	handler.indexFileName = "hospital.ind";
	handler.dataFileName = "hospital.dat";
	handler.doctorFileName = "doctor.dat";
	handler.dataIndexFile = fopen(handler.indexFileName, "ab");
	handler.doctorDataFile = fopen(handler.doctorFileName, "wb+");
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
		while (doc.nextDoctorID != docID) {
			doc = searchInSlaveFile(doc.nextDoctorID);
			printSlaveRecord(&doc);
		}
	}
	else {
		if (hospital.firstDoctorID != -1) {
			doc = searchInSlaveFile(hospital.firstDoctorID);
			while (doc.nextDoctorID != -1) {
				doc = searchInSlaveFile(doc.nextDoctorID);
				printSlaveRecord(&doc);
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
	}
}


void writeToSlaveFile(doctor* doctor) {
	
	handler.doctorDataFile = fopen(handler.doctorFileName, "ab");
	fwrite(doctor, sizeof(doctor), 1, handler.doctorDataFile);
	fclose(handler.doctorDataFile);
}

doctor searchInSlaveFile(int id) {
	doctor doc;
	handler.doctorDataFile = fopen(handler.doctorFileName, "rb");
	for (;;) {
		fread(&doc, sizeof(doc), 1, handler.doctorDataFile);
		if (feof(handler.doctorDataFile)) break;
		if (doc.id == id) return doc;
	}
	doc.id = -1;
}

void appendAfterLastDoctorInHospital(doctor* firstDoc, doctor* newDoc) {
	while (firstDoc->nextDoctorID != -1) {
		doctor doc = (searchInSlaveFile(firstDoc->nextDoctorID));
		firstDoc = &doc;
	}
	firstDoc->nextDoctorID = newDoc->id;
}

void printSlaveRecord(doctor* doc) {
	printf("ID: %d Name: %s Surname: %s DateOfBirth: %s Experience: %d\n",
		doc->id, doc->name, doc->surname, doc->dateOfBirth, doc->experience);
}