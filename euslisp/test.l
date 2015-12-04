(ros::load-ros-manifest "roseus")
(ros::roseus "zijia")
(defun wait-query (&optional (demo-type :map))
  (let ((atype nil))
    (ros::subscribe "/query_result" std_msgs::string
                    #'(lambda(m)
                        (print (list :subscribe (send m :data)))
                        (setq atype (send m :data))
                        ))
  (ros::rate 1)
    (while (null atype)
     (ros::ros-info " Waiting ....")
     (ros::spin-once)


     (ros::sleep))
    (cond
     ((or (string= atype "georgia") (string= atype "iemon") (string= atype "wonda"))
      ;; (demo :type atype :demo-type demo-type)
       (print "ready to go")
       (ros::spin-once)
       (setq atype nil from nil))
     (t))
))