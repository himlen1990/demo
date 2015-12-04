(define (domain pour)
        (:requirements :typing)
        (:types object)
    
        (:predicates
                (on ?obj - object ?table - location)
                (empty ?cup - object)
                (full ?cup - object)
                (find ?obj - object)
                (freehand)
                (step1)
                (ontable ?obj)
                )
        (:action pour 
        :parameters (?cup  - object ?teapot - object  ?table - location)
        :precondition (and
                      (step1)
                      (not (on ?teapot ?table))
                      (empty ?cup)
                      )
        :effect (and 
                (not (empty ?cup))
                  
                ))

        (:action pickup
        :parameters (?obj - object ?table - location )
        :precondition (and
                      (freehand)
                      (on ?obj ?table))
        :effect (and 
                (not (freehand))
                (step1)
                (not (on ?obj ?table))
                (not (on ?obj ?table))))
          )
                