(use-modules (lib) (curl))

(with-test-prefix
 "handle gc"
 (pass-if "doesn't crash"
	  (false-if-exception
	   (begin
	     (let loop ((h 0))
	       (if (< h 10000)
		   (begin
		     (let ((h (curl-easy-init)))
		       (curl-easy-setopt h 'url "http://localhost"))
		     (loop (1+ h)))
		   #t))))))
