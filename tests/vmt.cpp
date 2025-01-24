#include "cd_hook.h"
#include <iostream>
#include <string>

// Base class: Animal
class Animal {
public:
    Animal(std::string animalName) {
        name = animalName;
    }
    virtual ~Animal() {
    }

    virtual void sound() const {
        std::cout << name << " makes a sound." << "\n";
    }

    virtual void move() const {
        std::cout << name << " moves." << "\n";
    }

    virtual void eat(std::string food_name) {
        std::cout << name << " eats generic animal " << food_name << ".\n";
    }

    std::string name;
};

// Derived class: Dog (from Animal)
class Dog : public Animal {
public:
   Dog(std::string dogName) : Animal(dogName) {}

    void sound() const override {
        std::cout << name << " says Woof!" << "\n";
    }

    void move() const override {
        std::cout << name << " runs around happily!" << "\n";
    }

    void eat(std::string food_name) override {
        std::cout << name << " eats dog " << food_name << ".\n";
    }
};

// Derived class: Cat (from Animal)
class Cat : public Animal {
public:
    Cat(std::string catName) : Animal(catName) {}

    void sound() const override {
        std::cout << name << " says Meow!" << "\n";
    }

    void move() const override {
        std::cout << name << " slinks gracefully." << "\n";
    }

    void eat(std::string food_name) override {
        std::cout << name << " eats cat " << food_name << ".\n";
    }
};

void hook(void* obj) {
    std::cout << "Hooked function called for object: " << obj << "\n";
    std::cout << "The Animal is named: " << static_cast<Animal*>(obj)->name << "\n";
    std::cout << "Running another method for " << static_cast<Animal*>(obj)->name << "\n";
    static_cast<Animal*>(obj)->eat("croissant"); 
}

int main() {
    Animal* myAnimal = new Animal("Bobo");
    Animal* myDog = new Dog("Buddy");
    Animal* myCat = new Cat("Whiskers");

    std::cout << "####################\nRunning base class methods:\n####################\n";
    /* Using base class methods */
    std::cout << "Animal: " << myAnimal << "\n";
    myAnimal->sound();
    myAnimal->move();
    myAnimal->eat("food");

    std::cout << "\nDog: " << myDog << "\n";
    myDog->sound();
    myDog->move();
    myDog->eat("food");
    
    std::cout << "\nCat: " << myCat << "\n";
    myCat->sound();
    myCat->move();
    myCat->eat("food");

    std::cout << "\n####################\nHooking and running hooked methods:\n####################\n";
    /* VMT Hooks */
    std::cout << "\n\nAnimal: " << myAnimal << " sound method hook\n";
    cd_hook_ctx *ctxAnimalSound = ch_create_ctx((void*)myAnimal, (void*)hook);
    if (CD_HOOK_OK != ch_vmt(ctxAnimalSound, true, 2)) return 1;
    myAnimal->sound();
    if (CD_HOOK_OK != ch_destroy_ctx(ctxAnimalSound)) return 1;
    std::cout << "\n\nDog: " << myDog << " sound method hook\n";
    cd_hook_ctx *ctxDogSound = ch_create_ctx((void*)myDog, (void*)hook);
    if (CD_HOOK_OK != ch_vmt(ctxDogSound, true, 2)) return 1;
    myDog->sound();
    if (CD_HOOK_OK != ch_destroy_ctx(ctxDogSound)) return 1;

    std::cout << "\n\nCat: " << myCat << " move method hook\n";
    cd_hook_ctx *ctxCatMove = ch_create_ctx((void*)myCat, (void*)hook);
    if (CD_HOOK_OK != ch_vmt(ctxCatMove, true, 3)) return 1;
    myCat->move();
    if (CD_HOOK_OK != ch_destroy_ctx(ctxCatMove)) return 1;

    std::cout << "\n####################\nRunning unhooked methods:\n####################\n";
    /* Using hooked base class methods after unhooking */
    std::cout << "Animal: " << myAnimal << "\n";
    myAnimal->sound();

    std::cout << "\nDog: " << myDog << "\n";
    myDog->sound();
    
    std::cout << "\nCat: " << myCat << "\n";
    myCat->move();

    delete myAnimal;
    delete myDog;
    delete myCat;

    return 0;
}
