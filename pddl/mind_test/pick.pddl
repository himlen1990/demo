(define (domain pick)
        (:requirements :typing)
        (:types object location)
        (:constants obj_cup - object obj_teapot - object obj_sth - object)
        (:predicates
                (pic1objstate ?obj)
                (pic2objstate ?obj)
                (obj_in_view ?obj)
                (grasping ?obj)
                (handfree)
                (relative-place ?obj1 ?obj2)
                (on ?obj - object ?p - location)
                )
        (:functions 
                    (total-cost)
                    (find ?obj - object))
                
        (:action grasp
        :parameters (?obj)
        :precondition (and
                      (handfree)
                      (obj_in_view ?obj))
        :effect (and 
                (grasping ?obj) 
                (not (handfree))))
        
        (:action move-obj-random-place
        :parameters (?obj - object ?p1 - location ?p2 - location)
        :precondition (and 
                      (grasping ?obj)
                      (on ?obj ?p1))
        :effect  (on ?obj ?p2))
         
        (:action move-obj-relative-place
        :parameters(?obj1 - object ?obj2 - object)
        :precondition (and
                      (obj_in_view ?obj1)
                      (obj_in_view ?obj2)
                      (not (relative-place ?obj1 ?obj2))
                      (grasping ?obj1)) 
        :effect   (relative-place ?obj1 ?obj2))

        (:action find
        :parameters (?obj)
        :precondition (not (obj_in_view ?obj))
        :effect (and 
                (obj_in_view ?obj)
                (increase (total-cost) (find ?obj))
                ))
        
       
        (:action ac1_or_pour 
        :parameters ()
        :precondition (and
                      (grasping obj_teapot)
                      (relative-place obj_teapot obj_cup)
                      (pic1objstate obj_cup))
        :effect (pic2objstate obj_cup))
        
        (:action put_down
        :parameters (?obj)
        :precondition (and (grasping ?obj) (not (handfree)))
        :effect 
                (and
                (not (grasping ?obj))
                (handfree)))
         )
