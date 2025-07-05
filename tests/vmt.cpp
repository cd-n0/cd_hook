#include "cd_hook.h"
#include "test_common.h"
#include <cstdint>
#include <string>
#include <cassert>
#include <cstring>

/*
 * As I stated in the readme, the offsets of the methods can change depending
 * on the compiler and the version of the compiler used during compilation.
 * That being said from my testing I realized that every single compiler that I
 * used other than MSVC (clang++/g++/Apple clang) was 1 higher than MSVCs output
 * for this example. It does look reliable enough but I didn't test it on every
 * compiler on every OS/architectures (obviously :D)
 */
#ifndef _MSC_VER
#define VMT_OFFSET 1
#else
#define VMT_OFFSET 0
#endif

#define UNUSED(x) (void)x
#define BUFFER_SIZE 2048

static char out[BUFFER_SIZE];

// Base class: Animal
class Animal {
public:
    Animal(std::string animalName) : name(animalName) {}

    virtual ~Animal() {}

    virtual void sound() const {
        strcpy(out, (name + " makes a sound.").c_str());
    }

    virtual void move() const {
        strcpy(out, (name + " moves.").c_str());
    }

    virtual void eat(std::string food_name) const {
        strcpy(out, (name + " eats generic animal " + food_name + ".").c_str());
    }

    std::string name;
};

// Derived class: Dog (from Animal)
class Dog : public Animal {
public:
    Dog(std::string dogName) : Animal(dogName) {}

    void sound() const override {
        strcpy(out, (name + " says Woof!").c_str());
    }

    void move() const override {
        strcpy(out, (name + " runs around happily!").c_str());
    }

    void eat(std::string food_name) const override {
        strcpy(out, (name + " eats dog " + food_name + ".").c_str());
    }
};

// Derived class: Cat (from Animal)
class Cat : public Animal {
public:
    Cat(std::string catName) : Animal(catName) {}

    void sound() const override {
        strcpy(out, (name + " says Meow!").c_str());
    }

    void move() const override {
        strcpy(out, (name + " slinks gracefully.").c_str());
    }

    void eat(std::string food_name) const override {
        strcpy(out, (name + " eats cat " + food_name + ".").c_str());
    }
};

// Hooked functions
void animal_sound_hook(const Animal* obj) {
    strcpy(out, (obj->name + " is hooked and makes a strange sound!").c_str());
}

void dog_sound_hook(const Dog* obj) {
    obj->eat("food, but it's a bit awkward");
}

void cat_move_hook(const Cat* obj) {
    strcpy(out, (obj->name + " is hooked and moves differently!").c_str());
}

int main() {
    Animal* myAnimal = new Animal("Bobo");
    Dog* myDog = new Dog("Buddy");
    Cat* myCat = new Cat("Whiskers");

    /* BEFORE HOOKING */
    myAnimal->sound();
    assert(0 == strcmp(out, "Bobo makes a sound."));

    myDog->sound();
    assert(0 == strcmp(out, "Buddy says Woof!"));

    myCat->move();
    assert(0 == strcmp(out, "Whiskers slinks gracefully."));

    /* VMT HOOKS */
    ch_hook_ctx* animal_sound_ctx = ch_create_ctx((void*)myAnimal, (void*)animal_sound_hook);
    ch_vmt(animal_sound_ctx, 1 + VMT_OFFSET);
    myAnimal->sound();
    assert(0 == strcmp(out, "Bobo is hooked and makes a strange sound!"));

    ch_hook_ctx* dog_sound_ctx = ch_create_ctx((void*)myDog, (void*)dog_sound_hook);
    ch_vmt(dog_sound_ctx, 1 + VMT_OFFSET);
    myDog->sound();
    assert(0 == strcmp(out, "Buddy eats dog food, but it's a bit awkward."));

    ch_hook_ctx* cat_move_ctx = ch_create_ctx((void*)myCat, (void*)cat_move_hook);
    ch_vmt(cat_move_ctx, 2 + VMT_OFFSET);
    myCat->move();
    assert(0 == strcmp(out, "Whiskers is hooked and moves differently!"));

    /* UNHOOKING */
    ch_unhook(animal_sound_ctx);
    ch_unhook(dog_sound_ctx);
    ch_unhook(cat_move_ctx);

    myAnimal->sound();
    assert(0 == strcmp(out, "Bobo makes a sound."));

    myDog->sound();
    assert(0 == strcmp(out, "Buddy says Woof!"));

    myCat->move();
    assert(0 == strcmp(out, "Whiskers slinks gracefully."));

    /* REHOOKING */
    ch_vmt(animal_sound_ctx, 1 + VMT_OFFSET);
    myAnimal->sound();
    assert(0 == strcmp(out, "Bobo is hooked and makes a strange sound!"));

    ch_vmt(dog_sound_ctx, 1 + VMT_OFFSET);
    myDog->sound();
    assert(0 == strcmp(out, "Buddy eats dog food, but it's a bit awkward."));

    ch_vmt(cat_move_ctx, 2 + VMT_OFFSET);
    myCat->move();
    assert(0 == strcmp(out, "Whiskers is hooked and moves differently!"));

    ch_destroy_ctx(dog_sound_ctx, false);
    /* UNHOOKING */
    ch_destroy_ctx(animal_sound_ctx, true);
    ch_destroy_ctx(cat_move_ctx, true);

    myAnimal->sound();
    assert(0 == strcmp(out, "Bobo makes a sound."));

    myDog->sound();
    assert(0 == strcmp(out, "Buddy eats dog food, but it's a bit awkward."));

    myCat->move();
    assert(0 == strcmp(out, "Whiskers slinks gracefully."));

    delete myAnimal;
    delete myDog;
    delete myCat;

    return 0;
}
