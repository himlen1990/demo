(define (problem pour-water)
        (:domain pour)
        (:objects cup teapot - object table - location )
        (:init 
               (on teapot table)
               (on cup table)
               (freehand)
               (empty cup)
               )
        (:goal
                (and
                (on cup table)
               
                (not (on teapot table))
                (not (freehand))
                (not (empty cup))
                ))
  )