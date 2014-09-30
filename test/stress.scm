(use-modules (lib) (curl))

(with-test-prefix
 "handle gc"
 (pass-if "doesn't crash"
	  (false-if-exception
	   (begin
	     (let loop ((i 0))
	       (if (< i 100000)
		   (begin
		     (let ((h (curl-easy-init)))
		       (curl-easy-setopt h 'url "http://localhost")
                       (curl-easy-setopt h 'httpheader (list (make-string 100 #\a) (make-string 100 #\b)))
                       (loop (1+ i))))
		   #t))))))
