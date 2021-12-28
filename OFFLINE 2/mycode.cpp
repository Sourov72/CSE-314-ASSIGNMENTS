#include<bits/stdc++.h>
#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<unistd.h>
#include<stdint.h>

using namespace std;


//semaphore and mutex control sleep and wake up
pthread_mutex_t kioskmtx, *beltmtx, speckioskmtx, vipleftmtx, viprightmtx, priorvipmtx, boardmtx;
sem_t kiosksem, *beltsem, vipleftsem, viprightsem, specialkiosksem, boardsem;
int M, N, P, w, x, y, z;
int pid = 1;



class passenger{


    int id;
    bool vip = false;
    bool boardingpass = false;
    int time = 0;

public:

    void setId(int tid){
        id = tid;
    }

    void setVip(bool tvip){
        vip = tvip;
    }

    void setPass(bool pass){
        boardingpass = pass;
    }

    void assignTime(int t){
        time += t;
    }


    int getId(){
        return id;
    }

    bool getVip(){
       
        return vip;
    }

    bool getPass(){
        return boardingpass;
    }

    int getTime(){
        return time;
    }

};

queue<passenger> passengerq, beltq[3], vipleftq, viprightq, specialkioskq, boardq;

void * create_passenger(void * arg){


    while(1){
    
        if(pid <= 10){

            passenger p;
            p.setId(pid);
            pid++;
            p.setVip(rand() % 2);
            p.setPass(false);
            cout << "creating pass" << endl;
            pthread_mutex_lock(&kioskmtx);
            passengerq.push(p);
            pthread_mutex_unlock(&kioskmtx);
            sem_post(&kiosksem);

        }
   

    }
}


void show_passenger(){

    while(passengerq.size()) {
      cout << passengerq.front().getId() << endl;
      passengerq.pop();
   }



}



void * checkinKiosk(void * arg){

    // int id = *((int*) arg);

    int id;
    id = (intptr_t) arg;

    cout << "thread with " << id << " created" << endl;
    passenger p;

    while(1){

        
        sem_wait(&kiosksem);
        pthread_mutex_lock(&kioskmtx);
		
		p = passengerq.front();
        passengerq.pop();
		pthread_mutex_unlock(&kioskmtx); 
        sleep(6);
        p.assignTime(6);
        p.setPass(rand() % 2);
        // printf("in thread %d, passenger id %d\n",id, p.getId());
        

        cout << "in kiosk line thread passenger id: " << p.getId() << " checked in" << endl;
        if(p.getVip()){

            pthread_mutex_lock(&vipleftmtx);
            if(vipleftq.size() == 0) 
                pthread_mutex_lock(&priorvipmtx);
            vipleftq.push(p);

            pthread_mutex_unlock(&vipleftmtx);
            sem_post(&vipleftsem);

        }
        else{

            int randbelt = rand() % 3;
            pthread_mutex_lock(&beltmtx[randbelt]);
            beltq[randbelt].push(p);
        
            pthread_mutex_unlock(&beltmtx[randbelt]);
            sem_post(&beltsem[randbelt]);

        }

    }

}




void * leftright(void * arg){

    passenger p;

    while(1){

        sem_wait(&vipleftsem);
        pthread_mutex_lock(&vipleftmtx);

        p = vipleftq.front();
        vipleftq.pop();
        if(vipleftq.size() == 0)
            pthread_mutex_unlock(&priorvipmtx);
        pthread_mutex_unlock(&vipleftmtx);

        cout << "in vip line left-right thread passenger id: " << p.getId() << " checked in" << endl; 


        pthread_mutex_lock(&boardmtx);
        boardq.push(p);
        pthread_mutex_unlock(&boardmtx);

        sem_post(&boardsem);


    }


}


void * rightleft(void * arg){

    passenger p;

    while(1){

        sem_wait(&viprightsem);
        pthread_mutex_lock(&priorvipmtx);
        pthread_mutex_lock(&viprightmtx);
        p = viprightq.front();
        viprightq.pop();
        pthread_mutex_unlock(&viprightmtx);
        pthread_mutex_unlock(&priorvipmtx);

        cout << "in vip line right-left thread passenger id: " << p.getId() << " checked in" << endl;


        pthread_mutex_lock(&speckioskmtx);

        specialkioskq.push(p);

        pthread_mutex_unlock(&speckioskmtx);
        sem_post(&specialkiosksem);




    }

}



void * boarding(void * arg){

    passenger p;

    while(1){

        sem_wait(&boardsem);
        pthread_mutex_lock(&boardmtx);

        p = boardq.front();
        boardq.pop();

        pthread_mutex_unlock(&boardmtx);

        cout << "in board line thread passenger id: " << p.getId() << " checked in" << endl;


        if(p.getPass()){

            cout << "passenger id: " << p.getId() << " successfully boarded " << endl;
        }

        else{

            pthread_mutex_lock(&viprightmtx);
            viprightq.push(p);
            pthread_mutex_unlock(&viprightmtx);
            sem_post(&viprightsem);

        }



    }
    

}

void * returnback(void * arg){


    passenger p;

    while(1){

        sem_wait(&specialkiosksem);

        pthread_mutex_lock(&speckioskmtx);

        p = specialkioskq.front();
        specialkioskq.pop();

        pthread_mutex_unlock(&speckioskmtx);

        cout << "in speical kiosk thread passenger id: " << p.getId() << " checked in" << endl;

        p.setPass(rand() % 2);

        pthread_mutex_lock(&vipleftmtx);
        if(vipleftq.size() == 0) 
            pthread_mutex_lock(&priorvipmtx);
        vipleftq.push(p);

        pthread_mutex_unlock(&vipleftmtx);
        sem_post(&vipleftsem);

    }


}




void * securityCheck(void * arg){

    int id;
    passenger p;
    queue<passenger> tempsq;
    id = (intptr_t) arg;

    cout << "security thread with " << id << " created" << endl;


    while(1){

        
        sem_wait(&beltsem[id]);

        pthread_mutex_lock(&beltmtx[id]);
        int val = min((int)beltq[id].size(), P);

        // cout << "value for security check: thread id: " << id << ", " << val << endl;
    
        while(val > 0){

            p = beltq[id].front();
            beltq[id].pop();
            tempsq.push(p);
            // cout << "in security thread " << id << ", passenger id " << p.getId() << " checked in "<< endl;

            cout << "in security line thread passenger id: " << p.getId() << " checked in" << endl;

            if(val != 1)
                sem_wait(&beltsem[id]);

            val--;


        }

        pthread_mutex_unlock(&beltmtx[id]);
        sleep(6);

        while(tempsq.size()){

            p = tempsq.front();
            p.assignTime(6);
            tempsq.pop();

            pthread_mutex_lock(&boardmtx);

            boardq.push(p);
            pthread_mutex_unlock(&boardmtx);

            sem_post(&boardsem);



            // not sure if the time is constant increasing or max amount of time

        }



    }


}





int main(void)
{	


    // srand(time(0));
    // show_passenger();

    ifstream inputfile;
    fstream outputfile;
    
    inputfile.open("input.txt", ios::in);
    outputfile.open("output.txt", ios::out);

    if(!inputfile){
        cout << "no input file" << endl;
        return 0;
    }

    freopen("output.txt", "w", stdout);


    inputfile >> M;
    inputfile >> N;
    inputfile >> P;
    inputfile >> w;
    inputfile >> x;
    inputfile >> y;
    inputfile >> z;


    cout << M << " " << N << " " << P << " " << w << " " << x << " " << y << " " << z << endl;



    beltmtx = new pthread_mutex_t[N];
    beltsem = new sem_t[P];
    pthread_t kioskthreads[M], belthread[N], pasthread, vipleftthread, viprightthread, boardthread, specialkioskthread;
    pthread_mutex_init(&kioskmtx, NULL);
    pthread_mutex_init(&speckioskmtx, NULL);
    pthread_mutex_init(&vipleftmtx, NULL);
    pthread_mutex_init(&viprightmtx, NULL);
    pthread_mutex_init(&priorvipmtx, NULL);
    pthread_mutex_init(&boardmtx, NULL);
    sem_init(&kiosksem, 0, 0);
    sem_init(&vipleftsem, 0, 0);
    sem_init(&viprightsem, 0, 0);
    sem_init(&specialkiosksem, 0, 0);
    sem_init(&boardsem, 0, 0);
    for(int i = 0; i < N; i++){

        pthread_mutex_init(&beltmtx[i], NULL);
        // no of mtx

    }

    for(int i = 0; i < P; i++){

        sem_init(&beltsem[i], 0, 0);
        // no of passenger in belt
    }

	int rc;
	for(int t = 0; t < M ; ){
        

        // rc = pthread_create(&threads[t], NULL, checkinKiosk, (void*) &temp);
        rc = pthread_create(&kioskthreads[t], NULL, checkinKiosk, (void*) (intptr_t) t);
        cout << "In main: creating different kiosk thread : " <<  t << endl;

        if (rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }

        t++;
        
    }


    for(int t = 0; t < N; ){
        

        // rc = pthread_create(&threads[t], NULL, checkinKiosk, (void*) &temp);
        // no of belts here
        rc = pthread_create(&belthread[t], NULL, securityCheck , (void*) (intptr_t) t);
        cout << "In main: creating different security thread : " <<  t << endl;

        if (rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
        }

        t++;
    }


    pthread_create(&vipleftthread, NULL, leftright, NULL);
    cout << "In main: creating vipleftq thread " << endl;
    pthread_create(&viprightthread, NULL, rightleft, NULL);
    cout << "In main: creating viprightq thread " << endl;

    pthread_create(&specialkioskthread, NULL, returnback, NULL);
    cout << "In main: creating speicalkiosk thread " << endl;
    pthread_create(&boardthread, NULL, boarding, NULL);
    cout << "In main: creating boarding thread " << endl;

    rc = pthread_create(&pasthread, NULL, create_passenger, NULL);
    cout << "In main: creating different passenger thread " << endl;

    if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
    }

	while(1);
	return 0;
}
