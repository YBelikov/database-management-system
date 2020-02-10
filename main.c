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
	struct Doctor* next;
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
	char* indexFileName;
	char* dataFileName;
}fileHandler;

fileHandler handler;

void updateMasterFileRecord(char [MAX_LENGTH_OF_COMMAND], char*);
int searchForMasterIndex(int);
hospital hospitalWithIndex(int);
void changeHospitalRecord(hospital*, char*);
void updateRecord(int, hospital*);
void deleteEndOfLine(char*);



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
		return;
	}
}


int main(int argc, char** argv) {
	printf("This is a hospital database management programm\n");
	printf("You have next commands available for input:\n");
	printf("1.\"show-m\" - to look information about all hospital in database\n");
	printf("2.\"get-m ID\" - to look information about hospital with ID key\n");
	printf("3.\"show-s ID\" - to look information about all doctors in hospital with ID key\n");
	printf("4.\"get-s hospitalID docID\" - to look information about doctors with docID key in hospital with hospitalID key\n");
	printf("5.\"insert-m budget\" - to add new hospital with budget\n");
	
	handler.indexFileName = "hospital.ind";
	handler.dataFileName = "hospital.dat";
	handler.dataIndexFile = fopen(handler.indexFileName, "ab");

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