(ros::roseus-add-msgs "posedetection_msgs")

(load "pr2main.l")

(pr2-init t)

(send *irtviewer* :darw-objects)

(unless (boundp '*tfl*)
  (setq *tfl* (instance ros::transform-listener :init)))
(defun square (x) (* x x))
(defun compute-dis ()
  (let ((dis 1000)
        trans-base)
    (ros::rate 10)
    (while (> dis 200)
      
      (setq trans-base 
            (send *tfl* :lookup-transform
                  "/head_1"
                  "/right_hand_1"
                  (ros::time 0)
                  ))
      (setq pos (send (send trans-base :copy-worldcoords) :pos))
      (setq dis (sqrt (+ (square (elt pos 0)) (square (elt pos 1)) (square (elt pos 2))))))
    (print "dis <=200")
    (while (< dis 400)
      (setq trans-base 
            (send *tfl* :lookup-transform
                  "/head_1"
                  "/right_hand_1"
                  (ros::time 0)
                  ))
      (setq pos (send (send trans-base :copy-worldcoords) :pos))
      (setq dis (sqrt (+ (square (elt pos 0)) (square (elt pos 1)) (square (elt pos 2))))))
    (print "dis >=400")))

