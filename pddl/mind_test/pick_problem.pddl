(define (problem pick)
        (:domain pick)
        (:objects obj_teapot - object obj_cup - object p1 - location p2 - location obj_tray -object)
        (:init
                (not (obj_in_view obj_tray))       
                (not (relative-place obj_teapot obj_cup))
                (not (relative-place obj_cup obj_tray))
                (pic1objstate obj_cup)
                (handfree)
                (= (total-cost) 0)
                (= (find obj_tray) 100)
                (= (find obj_cup) 10)
                (= (find obj_teapot) 10)
                )
        (:goal 
               (and
               
               (relative-place obj_cup obj_tray)
               (pic2objstate obj_cup)
               
              
               (handfree))
               )
        (:metric minimize (total-cost))       
 )