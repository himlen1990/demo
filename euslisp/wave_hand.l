(load "package://pr2eus/pr2-interface.l")
(pr2-init)
(objects *pr2*)
(read-line)
(send *pr2* :larm :collar-y :joint-angle  90)
(send *pr2* :larm :shoulder-p :joint-angle -20)
(send *pr2* :larm :shoulder-r :joint-angle 0)
(send *pr2* :larm :elbow-r :joint-angle 90)
(send *ri* :angle-vector (send *pr2* :angle-vector) 4000)
(defun wave-hand ()
  (do ((i 0 (+ i 1)))
      ((> i 20) 'done)
      (progn
        (send *pr2* :larm :elbow-p :joint-angle -50)
        (send *ri* :angle-vector (send *pr2* :angle-vector) 4000)
        (send *pr2* :larm :elbow-p :joint-angle -100)
        (send *ri* :angle-vector (send *pr2* :angle-vector) 4000))))

      
