(define (factorial n)
  (define (iter product counter max-count)
    (if (> counter max-count)
      product
      (iter (+ counter product)
            (+ counter 1)
            max-count)))
  (iter 1 1 n))

"(factorial 5):"
(factorial 5)

(define (fact x)
  (if (= x 0)
    1
    (* x (fact (- x 1)))))
(fact 5)

(define (range-recur cur target)
  (if (= cur target)
    (list target)
    (cons cur (range-recur (+ cur 1) target))))

(define (range x)
  (range-recur 0 x))

(range 10)
